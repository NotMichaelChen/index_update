#ifndef STATIC_INDEX_HPP
#define STATIC_INDEX_HPP

#include <string>
#include <vector>

#include "global_types.hpp"
#include "sparse_lexicon.hpp"

/**
 * Responsible for writing and managing the static indexes on disk
 * Z-indexes are the main indexes whereas I-indexes are indexes of a similar order to the Z-indexes
 * which must then be merged with the Z-index to produce a higher-order index
 */
class StaticIndex {

public:
    StaticIndex(const std::string& workind_dir);

    void write_p_disk(PosMapIter indexbegin, PosMapIter indexend);
    void write_np_disk(NonPosMapIter indexbegin, NonPosMapIter indexend);

    SparseExtendedLexicon* getExLexPointer();

private:

    SparseExtendedLexicon spexlex;

    const std::string INDEXDIR;
    const std::string PDIR;
    const std::string NPDIR;

    //Writes an index (stored as a map of wordIDs to posting lists) to disk
    template <typename T>
    void write_index(std::string& indexname, std::ofstream& ofile, bool positional, T indexbegin, T indexend);

    //Checks whether there are any indexes that need to be merged (which is indicated by I-indexes)
    //and merges them until there are no more indexes to merge (no more I-indexes)
    void merge_test(bool isPositional);

    //Merges the indexes of the given order. Both the Z-index and I-index must already exist before
    //this method is called.
    void merge(int indexnum, bool positional);

    void shouldGetLexEntry(unsigned int postinglistsize, unsigned int termID, unsigned int indexnum, bool isZindex,
        size_t offset, bool positional, size_t& postingcount, bool& lastlisthadpointer);

    //Merges two posting lists into one posting list
    std::vector<Posting> merge_pos_postinglist(std::vector<Posting>& listz, std::vector<Posting>& listi);
    std::vector<nPosting> merge_nonpos_postinglist(std::vector<nPosting>& listz, std::vector<nPosting>& listi);
};

#endif