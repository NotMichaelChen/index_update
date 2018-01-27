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
    StaticIndex(std::string dir, int blocksize);

    void write_p_disk(std::map<unsigned int, std::vector<Posting>>::iterator indexbegin,
        std::map<unsigned int, std::vector<Posting>>::iterator indexend);

    void write_np_disk(std::map<unsigned int, std::vector<nPosting>>::iterator indexbegin,
        std::map<unsigned int, std::vector<nPosting>>::iterator indexend);

private:
    using Pos_Map_Iter = std::map<unsigned int, std::vector<Posting>>::iterator;
    using NonPos_Map_Iter = std::map<unsigned int, std::vector<nPosting>>::iterator;

    using Pos_Index = std::map<unsigned int, std::vector<Posting>>;
    using NonPos_Index = std::map<unsigned int, std::vector<nPosting>>;

    std::string indexdir;
    std::string posdir;
    std::string nonposdir;
    int blocksize;

    ExtendedLexicon exlex;

    //Writes a given block (vector) of compressed posting data into the file
    template <typename T>
    unsigned int write_block(std::vector<T> num, std::ofstream& ofile);

    //Compresses a vector of posting data using the given compression method
    std::vector<uint8_t> compress_block(std::vector<unsigned int>& field, std::vector<uint8_t> encoder(std::vector<unsigned int>&), bool delta);

    //Writes an index (stored as a map of wordIDs to posting lists) to disk
    template <typename T>
    void write_index(std::string filepath, std::ofstream& ofile, bool positional, T indexbegin, T indexend);

    //Decompresses the given positional static index
    Pos_Index decompress_p_posting(unsigned int termID, std::ifstream& ifile, std::string namebase);

    //Decompresses the given nonpositional static index
    NonPos_Index decompress_np_posting(unsigned int termID, std::ifstream& filez,
        std::ifstream& filei, std::string namebase1, std::string namebase2);

    //Checks whether there are any indexes that need to be merged (which is indicated by I-indexes)
    //and merges them until there are no more indexes to merge (no more I-indexes)
    void merge_test(bool isPositional);

    //Merges the indexes of the given order. Both the Z-index and I-index must already exist before
    //this method is called.
    void merge(int indexnum, int positional);

    //Merges two positional indexes
    //Used in the "merge" method
    Pos_Index merge_positional_index(Pos_Index& indexZ, Pos_Index& indexI);
};

#endif