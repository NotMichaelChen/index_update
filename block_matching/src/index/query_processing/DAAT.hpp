#ifndef DAAT_HPP
#define DAAT_HPP

#include <vector>
#include <unordered_map>

#include "index/sparse_lexicon.hpp"
#include "index/dynamic_index.hpp"
#include "global_parameters.hpp"
#include "Structures/documentstore.h"

//Returns the vector of docIDs that were found, from low-high
std::vector<unsigned int> DAAT(std::vector<unsigned int>& termIDs, std::vector<unsigned int>& docscontaining,
    DynamicIndex& index, SparseExtendedLexicon& exlex, std::string staticpath, DocumentStore& docstore);

#endif