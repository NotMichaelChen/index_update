#ifndef EXTLEX_H
#define EXTLEX_H

#include <string>
#include <unordered_map>
#include <vector>
#include "meta.hpp"

class ExtendedLexicon {
public:

    void addPositional(unsigned int term, mData& entry);
    void addNonPositional(unsigned int term, mData& entry);

    void displayNonPositional();

    mData getPositional(unsigned int term, std::string filename);
    mData getNonPositional(unsigned int term, std::string filename);

    std::vector<mData>::iterator getPositionalBegin(unsigned int term);
    std::vector<mData>::iterator getPositionalEnd(unsigned int term);
    std::vector<mData>::iterator getNonPositionalBegin(unsigned int term);
    std::vector<mData>::iterator getNonPositionalEnd(unsigned int term);

private:
    std::unordered_map<unsigned int, std::vector<mData>> exlexnp;
    std::unordered_map<unsigned int, std::vector<mData>> exlexp;
};

#endif
