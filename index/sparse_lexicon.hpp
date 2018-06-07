#ifndef SPARSE_LEXICON_HPP
#define SPARSE_LEXICON_HPP

#include <vector>
#include <unordered_map>

class SparseExtendedLexicon {

public:
    void insertEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset, bool positional);

    //Clears all of the entries inside of the given index (both Z-indexes and I-indexes)
    void clearIndex(unsigned int indexnum, bool positional);

    unsigned long getPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex);
    unsigned long getNonPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex);

    void printSize();

private:
    //Each vector element represents an indexnum with its own lexicon, which maps termIDs to offsets
    std::vector<std::unordered_map<unsigned int, unsigned long>> zposlex;
    std::vector<std::unordered_map<unsigned int, unsigned long>> znonposlex;

    std::vector<std::unordered_map<unsigned int, unsigned long>> iposlex;
    std::vector<std::unordered_map<unsigned int, unsigned long>> inonposlex;
};

#endif