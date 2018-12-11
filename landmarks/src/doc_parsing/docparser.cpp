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
    std::vector<EditEntry> transformedscript = transformDiff(se->getOldEncoded(), se->getNewEncoded(), landdir.getLandmarkArray(this->docID), diffscript);

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