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

    //Removes the given entry from the lexicon
    std::vector<mData>::iterator deletePositional(unsigned int term, std::vector<mData>::iterator entry);
    std::vector<mData>::iterator deleteNonPositional(unsigned int term, std::vector<mData>::iterator entry);

    std::vector<mData>::iterator getPositional(unsigned int term, std::string filename);
    std::vector<mData>::iterator getNonPositional(unsigned int term, std::string filename);

    std::vector<mData>::iterator getPositionalBegin(unsigned int term);
    std::vector<mData>::iterator getPositionalEnd(unsigned int term);
    std::vector<mData>::iterator getNonPositionalBegin(unsigned int term);
    std::vector<mData>::iterator getNonPositionalEnd(unsigned int term);

private:
    //Each term is associated with a list of metadata structs.
    //Each struct corresponds to that term's entry in a certain file.
    std::unordered_map<unsigned int, std::vector<mData>> exlexnp;
    std::unordered_map<unsigned int, std::vector<mData>> exlexp;
};

#endif
