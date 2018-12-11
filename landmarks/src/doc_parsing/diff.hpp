#pragma once

#include <vector>
#include <string>

struct DiffEntry {
    DiffEntry(bool i, unsigned int o, unsigned int n) : isIns(i), oldpos(o), newpos(n) {}
    //otherwise deletion
    bool isIns;
    //pos in the old document
    //insertion positions refer to "insert before this index"
    unsigned int oldpos;
    unsigned int newpos;
};

struct DiffRange {
    DiffRange(bool i, unsigned int s, unsigned int l, std::vector<int>& t) : isIns(i), start(s), len(l), terms(t) {}
    bool isIns;
    // Refers to positions in the olddoc
    unsigned int start;
    unsigned int len;
    std::vector<int> terms;
};

// Create a vector of diffranges that indicate a range of insertions or deletions
std::vector<DiffRange> makeDiffRange(const std::vector<int>& olddoc, const std::vector<int>& newdoc);
std::vector<DiffEntry> diff(std::vector<int> e, std::vector<int> f, int i = 0, int j = 0);