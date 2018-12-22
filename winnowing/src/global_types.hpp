#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

#include <utility>
#include <unordered_map>
#include <string>
#include <map>
#include <queue>
#include "index/posting.hpp"

typedef std::pair<size_t, size_t> FragID;

struct Fragment {
    // Fragment(size_t did, size_t offset, size_t fragsize) : docid(did), offset(offset), fragsize(fragsize) {}
    size_t docid;
    size_t startpos;
    std::string fragcontent;
};

// struct Posting;
// struct nPosting;

typedef std::map<unsigned int, std::vector<Posting>>::iterator PosMapIter;
typedef std::map<unsigned int, std::vector<nPosting>>::iterator NonPosMapIter;

typedef std::map<unsigned int, std::vector<Posting>> PosIndex;
typedef std::map<unsigned int, std::vector<nPosting>> NonPosIndex;

typedef std::priority_queue<double, std::vector<double>, std::greater<double>> DoubleMinHeap;

#endif