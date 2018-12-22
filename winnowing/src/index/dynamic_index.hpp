#pragma once

#include <cstdint>

#include "global_types.hpp"
#include "static_index.hpp"

class DynamicIndex {
public:
    void insertPostingP(unsigned int termid, unsigned int docid, unsigned int fragid, unsigned int pos);
    void insertPostingNP(unsigned int termid, unsigned int docid, unsigned int freq);

    std::pair<PosMapIter, PosMapIter> getPosIter();
    std::pair<NonPosMapIter, NonPosMapIter> getNonPosIter();

    uint64_t getPPostingCount();
    uint64_t getNPPostingCount();

    std::vector<nPosting>* getNPostingList(unsigned int termid);

    void dump(StaticIndex& staticwriter);
    // No restore method, just dumps its info onto disk
    void clear();
    void clearPos();
    void clearNonPos();

private:
    PosIndex positional_index;
    NonPosIndex nonpositional_index;

    uint64_t positional_size;
    uint64_t nonpositional_size;
};