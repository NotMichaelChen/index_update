#ifndef GLOBAL_PARAMETERS_HPP
#define GLOBAL_PARAMETERS_HPP

#define MIN_BLOCK_SIZE 15
#define MAX_BLOCK_COUNT 20

#define POSTING_LIMIT 50000000 //make sure doesn't exceed memory limit
#define BLOCKSIZE 128
#define INDEXDIR "./disk_index/"//path to static index folders
#define PDIR "./disk_index/positional/"//path to static positional index
#define NPDIR "./disk_index/non_positional/"//path to static non-positional index

#define DAAT_SIZE 10

#include <vector>
#include <map>
#include <queue>
#include <limits>

//Type Aliases

struct Posting;
struct nPosting;

namespace GlobalType {
    using PosMapIter = std::map<unsigned int, std::vector<Posting>>::iterator;
    using NonPosMapIter = std::map<unsigned int, std::vector<nPosting>>::iterator;

    using PosIndex = std::map<unsigned int, std::vector<Posting>>;
    using NonPosIndex = std::map<unsigned int, std::vector<nPosting>>;

    using DoubleMinHeap = std::priority_queue<double, std::vector<double>, std::greater<double>>;
}

//Constants

namespace GlobalConst {
    const auto UIntMax = std::numeric_limits<unsigned int>::max();
}

#endif