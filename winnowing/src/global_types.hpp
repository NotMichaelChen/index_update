#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

#include <utility>
#include <unordered_map>
#include <string>

typedef std::pair<size_t, size_t> FragID;

typedef std::unordered_map<std::string, size_t> PageTable;

struct Fragment {
    // Fragment(size_t did, size_t offset, size_t fragsize) : docid(did), offset(offset), fragsize(fragsize) {}
    size_t docid;
    size_t startpos;
    std::string fragcontent;
};

#endif