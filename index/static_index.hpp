#ifndef STATIC_INDEX_HPP
#define STATIC_INDEX_HPP

#include <string>
#include <map>
#include <vector>

#include "posting.hpp"

//Responsible for writing and managing the static indexes on disk
class StaticIndex {

public:
    StaticIndex(std::string dir, int blocksize);

    void write_p_disk(std::map<unsigned int, std::vector<Posting>>::iterator indexbegin,
        std::map<unsigned int, std::vector<Posting>>::iterator indexend);

    void write_np_disk(std::map<unsigned int, std::vector<nPosting>>::iterator indexbegin,
        std::map<unsigned int, std::vector<nPosting>>::iterator indexend);

private:
    std::string indexdir;
    std::string posdir;
    std::string nonposdir;
    int blocksize;

    template <typename T>
    void write(std::vector<T> num, std::ofstream& ofile);

    std::vector<uint8_t> compress_block(std::vector<unsigned int>& field, int method, int delta);

    template <typename T1, typename T2>
    void write_compressed_index(std::string namebase, std::ofstream& ofile, T1& ite, T1& end, T2& vit, T2& vend, int positional);

    void decompress_p_posting(unsigned int termID, std::ifstream& ifile, std::string namebase);

    void decompress_np_posting(unsigned int termID, std::ifstream& filez,
        std::ifstream& filei, std::string namebase1, std::string namebase2);

    void merge_test(bool isPositional);

    void merge(int indexnum, int positional);

};

#endif