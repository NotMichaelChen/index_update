#include "RAWreader.hpp"

#include <algorithm>

#include "util.hpp"

RAWReader::RAWReader(std::string dir) : docdir(dir), doc_index(0) {
    doc_collection = Utility::readDirectory(dir);
    std::sort(doc_collection.begin(), doc_collection.end());
    file.open("./" + dir + "/" + doc_collection[doc_index]);
    if(!file)
        throw std::runtime_error("Error opening file in document collection");

    //Read url from first header
    std::getline(file, nextline);
    auto tokens = Utility::splitString(nextline, ' ');
    if(tokens.size() != 3)
        throw std::runtime_error("Error, invalid document header in raw file");
    url = tokens[2];

    //Read first document
    std::string line;
    while(line.compare(0, 32, "df6fa1abb58549287111ba8d776733e9") != 0) {
        docbuffer += (line + "\n");
        std::getline(file, line);
    }

    //Store over-read line
    nextline = line;
}

std::string RAWReader::getCurrentDocument() {
    return docbuffer;
}

std::string RAWReader::getURL() {
    return url;
}

bool RAWReader::nextDocument() {
    if(doc_index >= doc_collection.size())
        return false;

    docbuffer.clear();

    //Get url from old header. Assume already have
    auto tokens = Utility::splitString(nextline, ' ');
    if(tokens.size() != 3)
        throw std::runtime_error("Error, invalid document header in raw file");
    url = tokens[2];

    //Read lines until magic number is found
    std::string line;
    std::getline(file, line);
    
    while(line.compare(0, 32, "df6fa1abb58549287111ba8d776733e9") != 0) {
        docbuffer += (line + "\n");
        std::getline(file, line);

        //Break on end-of-file
        if(!file)
            break;
    }
    
    //Store the over-read line
    nextline = line;

    if(!file) {
        file.close();
        file.clear();

        doc_index++;
        //Return true here since a valid document was still read, but we don't want to try reading the next file
        if(doc_index >= doc_collection.size())
            return true;
        
        file.open("./" + docdir + "/" + doc_collection[doc_index]);
        if(!file)
            throw std::runtime_error("Error opening file in document collection, " + doc_collection[doc_index]);
        
        //Read url from first header
        std::getline(file, nextline);
        auto tokens = Utility::splitString(nextline, ' ');
        if(tokens.size() != 3)
            throw std::runtime_error("Error, invalid document header in raw file");
        url = tokens[2];

        //Read first document
        std::string line;
        while(line.compare(0, 32, "df6fa1abb58549287111ba8d776733e9") != 0) {
            docbuffer += (line + "\n");
            std::getline(file, line);
        }

        //Store over-read line
        nextline = line;
    }

    return true;
}

bool RAWReader::isValid() {
    return doc_index < doc_collection.size();
}