#ifndef RAWREADER_HPP
#define RAWREADER_HPP

#include <vector>
#include <string>
#include <fstream>

#include "reader_interface.hpp"

class RAWReader : public ReaderInterface {
public:
    //Can only read directories in the same directory as the executable TODO: Make directory support better
    RAWReader(std::string dir);

    std::string getURL();
    std::string getCurrentDocument();
    bool nextDocument();
    bool isValid();

private:
    std::string docdir;
    std::vector<std::string> doc_collection;
    size_t doc_index;

    std::ifstream file;

    std::string docbuffer;
    std::string url;
    //Stores the line associated with the next document
    std::string nextline;
};

#endif