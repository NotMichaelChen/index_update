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

    mDatap getPositional(unsigned int term, std::string index); //TODO: change the method
    mDatanp getNonPositional(unsigned int term, int index);

    std::vector<mDatap>::iterator getPositionalBegin(unsigned int term);
    std::vector<mDatap>::iterator getPositionalEnd(unsigned int term);
    std::vector<mDatanp>::iterator getNonPositionalBegin(unsigned int term);
    std::vector<mDatanp>::iterator getNonPositionalEnd(unsigned int term);

private:
    std::unordered_map<unsigned int, std::vector<mData>> exlexnp;
    std::unordered_map<unsigned int, std::vector<mData>> exlexp;
};

#endif
