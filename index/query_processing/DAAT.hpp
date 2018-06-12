#ifndef DAAT_HPP
#define DAAT_HPP

#include <vector>
#include <unordered_map>

#include "sparse_lexicon.hpp"
#include "global_parameters.hpp"

//Statistical data from the index used during first-stage query processing
struct DAATStatData {
    unsigned int totaldocs;
    //Docs containing term t
    std::vector<unsigned int>* docscontaining;
    //Frequency of term t in document d found during DAAT
    //Length of doc d
    std::unordered_map<unsigned int, unsigned int>* doclengths;
    //Average length of documents in index
    double avgdoclength;
};

//Returns the vector of docIDs that were found, from low-high
std::vector<unsigned int> DAAT(std::vector<unsigned int>& termIDs, GlobalType::NonPosIndex& index, SparseExtendedLexicon& exlex,
    std::string staticpath, DAATStatData statistics);

#endif