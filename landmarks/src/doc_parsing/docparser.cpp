#include "docparser.hpp"

#include "diff.hpp"
#include "transform.hpp"

DocumentParser::DocumentParser(std::string& url, std::string& newpage, std::string& timestamp, DocumentStore& docstore, LandmarkDirectory& landdir) {
    DocumentTuple olddoc = docstore.getDocument(url);
    
    if(olddoc.doc.empty()) {
        olddoc.docID = docstore.getNextDocID();
        this->docID = olddoc.docID;
    }

    // TODO: handle new document case

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
    LandmarkArray& landarray, std::vector<DiffRange>& editscript)
{
    std::vector<EditEntry> transformedscript;

    for(DiffRange entry : editscript) {
        auto landiter = landarray.getLandmark(entry.start);
        
        if(entry.isIns) {
            // Shift every landmark entry.len to the right
            // We always shift every landmark after the current one
            auto nextlanditer = landarray.getNextLandmark(landiter);
            if(nextlanditer != landarray.getEnd()) {
                landarray.shiftLandmarks(nextlanditer, entry.len);
            }
            unsigned int posend = (nextlanditer == landarray.getEnd()) ? olddoc.size() : nextlanditer->pos;

            // Invalidate old landmark and insert new one
            landarray.refreshLandmark(landiter);
            unsigned int newlandmarkid = landiter->landID;

            // If associated landmark with range overlaps range beginning
            // Don't subtract 1 since this means insertions happen at beginning of landmark
            if(landiter->pos == entry.start) {
                unsigned int posinlandmark = 0;
                // Issue new postings starting with new words then old words in the landmark
                for(int i : entry.terms) {
                    pospostings.emplace_back(se->decodeNum(i), docID, newlandmarkid, posinlandmark);
                    posinlandmark++;
                }
                for(unsigned int i = landiter->pos; i < posend; i++) {
                    pospostings.emplace_back(se->decodeNum(olddoc[i]), docID, newlandmarkid, posinlandmark);
                    posinlandmark++;
                }
            }
            else {
                unsigned int posinlandmark = 0;
                // Issue old postings until insertion range
                for(unsigned int i = landiter->pos; i < entry.start; i++) {
                    pospostings.emplace_back(se->decodeNum(olddoc[i]), docID, newlandmarkid, posinlandmark);
                    posinlandmark++;
                }
                // Issue new postings
                for(int i : entry.terms) {
                    pospostings.emplace_back(se->decodeNum(i), docID, newlandmarkid, posinlandmark);
                    posinlandmark++;
                }
                // Issue old postings until end of landmark
                for(unsigned int i = entry.start; i < posend; i++) {
                    pospostings.emplace_back(se->decodeNum(olddoc[i]), docID, newlandmarkid, posinlandmark);
                    posinlandmark++;
                }
            }
        }
        else {
            // Invalidate all landmarks that are part of the deletion range
            auto overlappedlandmarks = landarray.getLandmarkRange(entry.start, entry.start + entry.len);
            if(overlappedlandmarks.empty())
                throw std::runtime_error("Error: landmark range in deletion is empty");

            landarray.deleteLandmarks(overlappedlandmarks);

            // Shift all landmarks past the deletion range
            auto nextlanditer = landarray.getLandmarkAfter(entry.start + entry.len);
            if(nextlanditer != landarray.getEnd()) {
                landarray.shiftLandmarks(nextlanditer, -entry.len);
            }

            // Reinsert landmark at position of first landmark that was invalidated
            auto firstinvalidlandmark = overlappedlandmarks[0]; // Guaranteed to exist, otherwise we'd throw exception
            unsigned int newlandmarkid = landarray.insertLandmark(firstinvalidlandmark->pos);


            // Reinsert terms before deletion range then after deletion range until next landmark
            unsigned int posinlandmark = 0;
            for(unsigned int i = firstinvalidlandmark->pos; i < entry.start; i++) {
                pospostings.emplace_back(se->decodeNum(olddoc[i]), docID, newlandmarkid, posinlandmark);
                ++posinlandmark;
            }

            // represents the landmark after the end of the deletion block
            unsigned int posend = (nextlanditer == landarray.getEnd()) ? olddoc.size() : nextlanditer->pos;

            for(unsigned int i = entry.start+entry.len; i < posend; i++) {
                pospostings.emplace_back(se->decodeNum(olddoc[i]), docID, newlandmarkid, posinlandmark);
                ++posinlandmark;
            }
        }
    }

    // TODO: fill in nonpositional data
}