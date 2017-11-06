#ifndef EXTLEX_H
#define EXTLEX_H

#include <string>
#include <unordered_map>
#include <vector>
#include "meta.hpp"

class ExtendedLexicon {
public:
    void addPositional(std::string term, mDatap& entry);
    void addNonPositional(std::string term, mDatanp& entry);
    
    mDatap getPositional(std::string term, int index);
    mDatanp getNonPositional(std::string term, int index);
private:
    std::unordered_map<std::string, std::vector<mDatanp>> exlexnp;
    std::unordered_map<std::string, std::vector<mDatap>> exlexp;
};

#endif
