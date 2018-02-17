#ifndef STATIC_INDEX_HPP
#define STATIC_INDEX_HPP

#include <string>
#include <map>
#include <vector>
#include <functional>

#include "posting.hpp"
#include "extended_lexicon.hpp"

/**
 * Responsible for writing and managing the static indexes on disk
 * Z-indexes are the main indexes whereas I-indexes are indexes of a similar order to the Z-indexes
 * which must then be merged with the Z-index to produce a higher-order index
 */
class StaticIndex {

public:
    void write_p_disk(std::map<unsigned int, std::vector<Posting>>::iterator indexbegin,
        std::map<unsigned int, std::vector<Posting>>::iterator indexend);

    void write_np_disk(std::map<unsigned int, std::vector<nPosting>>::iterator indexbegin,
        std::map<unsigned int, std::vector<nPosting>>::iterator indexend);

private:
    using Pos_Map_Iter = std::map<unsigned int, std::vector<Posting>>::iterator;
    using NonPos_Map_Iter = std::map<unsigned int, std::vector<nPosting>>::iterator;

    using Pos_Index = std::map<unsigned int, std::vector<Posting>>;
    using NonPos_Index = std::map<unsigned int, std::vector<nPosting>>;

    ExtendedLexicon exlex;

    //Writes an index (stored as a map of wordIDs to posting lists) to disk
    template <typename T>
    void write_index(std::string& filepath, std::ofstream& ofile, bool positional, T indexbegin, T indexend);

    //Checks whether there are any indexes that need to be merged (which is indicated by I-indexes)
    //and merges them until there are no more indexes to merge (no more I-indexes)
    void merge_test(bool isPositional);

    //Merges the indexes of the given order. Both the Z-index and I-index must already exist before
    //this method is called.
    void merge(int indexnum, bool positional);

    //Merges two posting lists into one posting list
    std::vector<Posting> merge_pos_postinglist(std::vector<Posting>& listz, std::vector<Posting>& listi);
    std::vector<nPosting> merge_nonpos_postinglist(std::vector<nPosting>& listz, std::vector<nPosting>& listi);
};

#endif