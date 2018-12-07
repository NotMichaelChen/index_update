#ifndef SPARSE_LEXICON_HPP
#define SPARSE_LEXICON_HPP

#include <vector>
#include <map>
#include <fstream>

class SparseExtendedLexicon {

public:
    void insertEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset, bool positional);

    //Clears all of the entries inside of the given index (both Z-indexes and I-indexes)
    void clearIndex(unsigned int indexnum, bool positional);

    unsigned long getPosLEQOffset(unsigned int termID, unsigned int indexnum, bool isZindex);
    unsigned long getNonPosLEQOffset(unsigned int termID, unsigned int indexnum, bool isZindex);

    void dump(std::ofstream& ofile);
    void restore(std::ifstream& ifile);
    void clear();
    void printSize();

private:
    //Each vector element represents an indexnum with its own lexicon, which maps termIDs to offsets
    std::vector<std::map<unsigned int, unsigned long>> zposlex;
    std::vector<std::map<unsigned int, unsigned long>> znonposlex;

    std::vector<std::map<unsigned int, unsigned long>> iposlex;
    std::vector<std::map<unsigned int, unsigned long>> inonposlex;
};

#endif