#include "WETreader.hpp"

#include <algorithm>
#include <iostream>

#include "util.hpp"

WETReader::WETReader(std::string dir) : doc_index(0) {
    doc_collection = Utility::readDirectory(dir);
    std::sort(doc_collection.begin(), doc_collection.end());

    file.open("./" + dir + "/" + doc_collection[0]);
    if(!file)
        throw std::runtime_error("Error opening file in document collection");
    
    //Skip first 18 lines
    std::string line;
    for(int i = 0; i < 18; i++)
        std::getline(file, line);

    nextDocument();
}

std::string WETReader::getCurrentDocument() {
    return docbuffer;
}

void WETReader::nextDocument() {
    header.clear();
    //Read header
    while(header.size() < 9) {
        std::string line;
        if(!std::getline(file, line))
            throw std::runtime_error("Error getting header in document");
        
        std::cerr << line << std::endl;

        header.push_back(line);
    }

    //Get number of chars to read
    std::vector<std::string> splitstr = Utility::splitString(header[8], ':');
    if(splitstr.size() != 2)
        throw std::runtime_error("Error parsing WET header");
    
    int docsize = std::stoi(splitstr[1]);

    docbuffer.clear();
    file.read(&docbuffer[0], docsize);
}