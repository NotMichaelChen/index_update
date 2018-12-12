#include "docparser.hpp"

#include "diff.hpp"
#include "transform.hpp"

DocumentParser::DocumentParser(std::string& url, std::string& newpage, std::string& timestamp, DocumentStore& docstore, LandmarkDirectory& landdir) {
    DocumentTuple olddoc = docstore.getDocument(url);
    
    if(olddoc.doc.empty()) {
        olddoc.docID = docstore.getNextDocID();
        this->docID = olddoc.docID;
    }

    se = std::make_unique<StringEncoder>(olddoc.doc, newpage);

    std::vector<DiffRange> diffscript = makeDiffRange(se->getOldEncoded(), se->getNewEncoded());
    // std::vector<EditEntry> transformedscript = transformDiff(se->getOldEncoded(), se->getNewEncoded(), landdir.getLandmarkArray(this->docID), diffscript);
    this->applyDiff(se->getOldEncoded(), se->getNewEncoded(), landdir.getLandmarkArray(this->docID), diffscript);

    docstore.insertDocument(url, newpage, se->getNewSize(), timestamp);
}

bool DocumentParser::isFirstDoc() {
    return se->getOldSize() == 0;
}

bool DocumentParser::termInOld(std::string& term) {
    return se->inOld(term);
}

bool DocumentParser::termInNew(std::string& term) {
    return se->inNew(term);
}

std::vector<ExternPposting> DocumentParser::getPPostings() {

}

std::vector<ExternNPposting> DocumentParser::getNPPostings() {

}

std::vector<Landmark> DocumentParser::getLandmarks() {

}

unsigned int DocumentParser::getDocID() {
    return docID;
}

void DocumentParser::applyDiff(const std::vector<int>& olddoc, const std::vector<int>& newdoc,
    LandmarkArray& landarray, std::vector<DiffRange>& editscript) {
    // TODO: Actually perform the operations so we have access to the latest landmarks
    std::vector<EditEntry> transformedscript;

    for(DiffRange entry : editscript) {
        auto landiter = landarray.getLandmark(entry.start);
        
        if(entry.isIns) {
            // Shift every landmark entry.len to the right
            // We always shift every landmark after the current one
            auto nextlanditer = landarray.getNextLandmark(landiter);
            if(nextlanditer != landarray.getEnd()) {
                transformedscript.emplace_back(true, EditEntry::shift, nextlanditer->landID, entry.len, -1);
            }
            unsigned int posend = (nextlanditer == landarray.getEnd()) ? olddoc.size() : nextlanditer->pos;

            // Invalidate old landmark and insert new one
            transformedscript.emplace_back(true, EditEntry::deletion, landiter->landID, 0, -1);
            unsigned int newlandmarkid = landarray.getAndIncrNextID();
            transformedscript.emplace_back(true, EditEntry::insertion, newlandmarkid, landiter->pos, -1);

            // If associated landmark with range overlaps range beginning
            // Don't subtract 1 since this means insertions happen at beginning of landmark
            if(landiter->pos == entry.start) {
                unsigned int posinlandmark = 0;
                // Issue new postings starting with new words then old words in the landmark
                for(int i : entry.terms) {
                    transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, posinlandmark, i);
                    posinlandmark++;
                }
                for(unsigned int i = landiter->pos; i < posend; i++) {
                    transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, posinlandmark, olddoc[i]);
                    posinlandmark++;
                }
            }
            else {
                unsigned int posinlandmark = 0;
                // Issue old postings until insertion range
                for(unsigned int i = landiter->pos; i < entry.start; i++) {
                    transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, posinlandmark, olddoc[i]);
                    posinlandmark++;
                }
                // Issue new postings
                for(int i : entry.terms) {
                    transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, posinlandmark, i);
                    posinlandmark++;
                }
                // Issue old postings until end of landmark
                for(unsigned int i = entry.start; i < posend; i++) {
                    transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, posinlandmark, olddoc[i]);
                    posinlandmark++;
                }
            }
        }
        else {
            // Invalidate all landmarks that are part of the deletion range
            auto overlappedlandmarks = landarray.getLandmarkRange(entry.start, entry.start + entry.len);
            if(overlappedlandmarks.empty())
                throw std::runtime_error("Error: landmark range in deletion is empty");

            for(auto iter : overlappedlandmarks) {
                transformedscript.emplace_back(true, EditEntry::deletion, iter->landID, 0, -1);
            }

            // Shift all landmarks past the deletion range
            auto nextlanditer = landarray.getLandmarkAfter(entry.start + entry.len);
            if(nextlanditer != landarray.getEnd()) {
                // Only need to add one entry since it is implied that the other landmarks will be shifted
                transformedscript.emplace_back(true, EditEntry::shift, nextlanditer->landID, -entry.len, -1);
            }

            // Reinsert landmark at position of first landmark that was invalidated
            auto firstinvalidlandmark = overlappedlandmarks[0]; // Guaranteed to exist, otherwise we'd throw exception
            unsigned int newlandmarkid = landarray.getAndIncrNextID();
            transformedscript.emplace_back(true, EditEntry::insertion, newlandmarkid, firstinvalidlandmark->pos, -1);

            // Reinsert terms before deletion range then after deletion range until next landmark
            for(unsigned int i = firstinvalidlandmark->pos; i < entry.start; i++) {
                transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, i - firstinvalidlandmark->pos, olddoc[i]);
            }

            // represents the landmark after the end of the deletion block
            unsigned int posend = (nextlanditer == landarray.getEnd()) ? olddoc.size() : nextlanditer->pos;

            for(unsigned int i = entry.start+entry.len; i < posend; i++) {
                transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, firstinvalidlandmark->pos, olddoc[i]);
            }
        }
    }
}