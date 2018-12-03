#ifndef WETREADER_HPP
#define WETREADER_HPP

#include <vector>
#include <string>
#include <fstream>

#include "reader_interface.hpp"

class WETReader : public ReaderInterface {
public:
    //Can only read directories in the same directory as the executable TODO: Make directory support better
    WETReader(std::string dir);

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
    //Header is always 9 lines
    std::vector<std::string> header;
};

#endif