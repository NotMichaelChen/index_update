#ifndef SPARSE_LEXICON_HPP
#define SPARSE_LEXICON_HPP

#include <vector>
#include <unordered_map>

class SparseExtendedLexicon {

public:
    void insertPosEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset);
    void insertNonPosEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset);

    unsigned long getPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex);
    unsigned long getNonPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex);

private:
    //Each vector element represents an indexnum with its own lexicon, which maps termIDs to offsets
    std::vector<std::unordered_map<unsigned int, unsigned long>> zposlex;
    std::vector<std::unordered_map<unsigned int, unsigned long>> znonposlex;

    std::vector<std::unordered_map<unsigned int, unsigned long>> iposlex;
    std::vector<std::unordered_map<unsigned int, unsigned long>> inonposlex;
};

#endif