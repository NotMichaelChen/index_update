#ifndef DAAT_HPP
#define DAAT_HPP

#include <queue>
#include <vector>
#include <map>

#include "query_primitive.hpp"
#include "../posting.hpp"
#include "../extended_lexicon.hpp"

std::priority_queue<double, std::vector<double>, std::greater<double>> DAAT(std::vector<unsigned int> docIDs, 
    std::map<unsigned int, std::vector<nPosting>>& index, ExtendedLexicon& exlex);

#endif