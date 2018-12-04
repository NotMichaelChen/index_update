#include "docparser.hpp"

#include "diff.hpp"
#include "transform.hpp"

DocumentParser::DocumentParser(std::string& url, std::string& newpage, std::string& timestamp, DocumentStore& docstore) {
    DocumentTuple olddoc = docstore.getDocument(url);
    
    if(olddoc.doc.empty()) {
        olddoc.docID = docstore.getNextDocID();
        this->docID = olddoc.docID;
    }

    se = std::make_unique<StringEncoder>(olddoc.doc, newpage);

    std::vector<DiffEntry> edittranscript = diff(se->getOldEncoded(), se->getNewEncoded());
    //TODO: maybe separate comparison into own function
    // std::sort(edittranscript.begin(), edittranscript.end(),
    //     [](const DiffEntry& lhs, const DiffEntry& rhs) {
    //         (lhs.pos == rhs.pos) ? (!lhs.isIns && rhs.isIns) : (lhs.pos < rhs.pos);
    //     }
    // );
    // transform(se->getOldEncoded(), , edittranscript);

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