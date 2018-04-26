#ifndef WETREADER_HPP
#define WETREADER_HPP

#include <vector>
#include <string>
#include <fstream>

#include "reader_interface.hpp"

class WETReader : public ReaderInterface {
public:
    WETReader(std::string dir);
    std::string getCurrentDocument();
    void nextDocument();

private:
    std::vector<std::string> doc_collection;
    size_t doc_index;

    std::ifstream file;
};

#endif