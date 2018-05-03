#include "WETreader.hpp"

#include <algorithm>
#include <iostream>

#include "util.hpp"

WETReader::WETReader(std::string dir) : docdir(dir), doc_index(0) {
    doc_collection = Utility::readDirectory(dir);
    std::sort(doc_collection.begin(), doc_collection.end());
    file.open("./" + dir + "/" + doc_collection[doc_index]);
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

std::string WETReader::getURL() {
    if(doc_index >= doc_collection.size())
        return "";

    size_t splitloc = header[2].find_first_of(':', 0);
    if(splitloc == std::string::npos)
        throw std::runtime_error("Error parsing WET header");

    //Advance beyond the first colon
    splitloc++;

    return Utility::trim(header[2].substr(splitloc, header[2].size()-splitloc));
}

bool WETReader::nextDocument() {
    if(doc_index >= doc_collection.size())
        return false;

    header.clear();

    std::string line;
    //Read header
    while(header.size() < 9) {
        if(!std::getline(file, line))
            throw std::runtime_error("Error getting header in nextDocument");

        header.push_back(line);
    }

    //Throw away line after header
    std::getline(file, line);

    //Get number of bytes to read
    std::vector<std::string> splitstr = Utility::splitString(header[8], ':');
    if(splitstr.size() != 2)
        throw std::runtime_error("Error parsing WET header");
    
    int docsize = std::stoi(splitstr[1]);

    std::vector<char> buf(docsize);
    docbuffer.clear();
    file.read(&buf.front(), docsize);

    docbuffer = std::string(buf.begin(), buf.end());

    //Throw away two lines after document
    std::getline(file, line);
    std::getline(file, line);
    //Read extra newline
    file.get();

    //If next char is invalid, begin reading next file
    if(file.peek() < 0) {
        file.close();
        file.clear();

        doc_index++;
        if(doc_index >= doc_collection.size())
            return false;
        
        file.open("./" + docdir + "/" + doc_collection[doc_index]);
        if(!file)
            throw std::runtime_error("Error opening file in document collection, " + doc_collection[doc_index]);
        
        //Skip first 18 lines
        std::string line;
        for(int i = 0; i < 18; i++)
            std::getline(file, line);
    }

    return true;
}