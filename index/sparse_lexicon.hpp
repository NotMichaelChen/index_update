#ifndef SPARSE_LEXICON_HPP
#define SPARSE_LEXICON_HPP

#include <vector>
#include <unordered_map>

struct ExtendedLexiconEntry {
    ExtendedLexiconEntry() : offset(0), length(0) {}
    ExtendedLexiconEntry(unsigned long o, unsigned int l) : offset(o), length(l) {}

    unsigned long offset;
    unsigned int length;
};

class SparseExtendedLexicon {

public:
    void insertPosEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset, unsigned int length);
    void insertNonPosEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset, unsigned int length);

    unsigned long getPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex);
    unsigned long getNonPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex);

private:
    //Each vector element represents an indexnum with its own lexicon, which maps termIDs to offsets
    std::vector<std::unordered_map<unsigned int, ExtendedLexiconEntry>> zposlex;
    std::vector<std::unordered_map<unsigned int, ExtendedLexiconEntry>> znonposlex;

    std::vector<std::unordered_map<unsigned int, ExtendedLexiconEntry>> iposlex;
    std::vector<std::unordered_map<unsigned int, ExtendedLexiconEntry>> inonposlex;
};

#endif