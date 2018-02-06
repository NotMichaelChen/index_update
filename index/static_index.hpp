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
    unsigned int write_block(std::vector<T>& num, std::ofstream& ofile);

    //Compresses a vector of posting data using the given compression method
    std::vector<uint8_t> compress_block(std::vector<unsigned int>& field, std::vector<uint8_t> encoder(std::vector<unsigned int>&), bool delta);

    //Writes an index (stored as a map of wordIDs to posting lists) to disk
    template <typename T>
    void write_index(std::string& filepath, std::ofstream& ofile, bool positional, T indexbegin, T indexend);

    //Writes a posting list to disk with compression
    template <typename T>
    void write_postinglist(std::ofstream& ofile, std::string& filepath, unsigned int termID, std::vector<T>& postinglist, bool positional);

    //Reads a posting list from disk
    std::vector<Posting> read_pos_postinglist(std::ifstream& ifile, std::vector<mData>::iterator metadata, unsigned int termID);
    std::vector<nPosting> read_nonpos_postinglist(std::ifstream& ifile, std::vector<mData>::iterator metadata, unsigned int termID);

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