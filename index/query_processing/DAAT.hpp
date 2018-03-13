#ifndef DAAT_HPP
#define DAAT_HPP

#include <queue>
#include <vector>
#include <map>

#include "query_primitive.hpp"
#include "../posting.hpp"
#include "../extended_lexicon.hpp"
#include "../global_parameters.hpp"

GlobalType::DoubleMinHeap DAAT(std::vector<unsigned int> docIDs, GlobalType::NonPosIndex& index, ExtendedLexicon& exlex);

#endif