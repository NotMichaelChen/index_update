#include "WETreader.hpp"

#include <algorithm>

#include "util.hpp"

WETReader::WETReader(std::string dir) : doc_index(0) {
    doc_collection = Utility::readDirectory(dir);
    std::sort(doc_collection.begin(), doc_collection.end());

    file.open(doc_collection[0]);
    if(!file)
        throw std::runtime_error("Error opening file in document collection");
}

std::string WETReader::getCurrentDocument() {
    return "";
}

void WETReader::nextDocument() {

}