#ifndef EXTLEX_H
#define EXTLEX_H

#include <string>
#include <map>
#include <vector>
#include "meta.hpp"

class ExtendedLexicon {
public:
    void addPositional(std::string term, mData entry);
    void addNonPositional(std::string term, mData entry);
private:
    std::map<std::string, std::vector<mData>> exlexnp;
    std::map<std::string, std::vector<mData>> exlexp;
};

#endif
