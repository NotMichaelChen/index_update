#include "dynamic_index.hpp"

#include "posting.hpp"

void DynamicIndex::insertPostingP(unsigned int termid, unsigned int docid, unsigned int landmarkid, unsigned int offset) {
    positional_index[termid].emplace_back(termid, docid, landmarkid, offset);
    positional_size++;
}

void DynamicIndex::insertPostingNP(unsigned int termid, unsigned int docid, unsigned int freq) {
    nonpositional_index[termid].emplace_back(termid, docid, freq);
    nonpositional_size++;
}

std::pair<GlobalType::PosMapIter, GlobalType::PosMapIter> DynamicIndex::getPosIter() {
    return std::make_pair(positional_index.begin(), positional_index.end());
}

std::pair<GlobalType::NonPosMapIter, GlobalType::NonPosMapIter> DynamicIndex::getNonPosIter() {
    return std::make_pair(nonpositional_index.begin(), nonpositional_index.end());
}

uint64_t DynamicIndex::getPPostingCount() {
    return positional_size;
}
uint64_t DynamicIndex::getNPPostingCount() {
    return nonpositional_size;
}

std::vector<nPosting>* DynamicIndex::getNPostingList(unsigned int termid) {
    return &nonpositional_index[termid];
}

void DynamicIndex::dump(StaticIndex& staticwriter) {
    staticwriter.write_np_disk(nonpositional_index.begin(), nonpositional_index.end());
    staticwriter.write_p_disk(positional_index.begin(), positional_index.end());
    clear();
}

void DynamicIndex::clear() {
    clearPos();
    clearNonPos();
}

void DynamicIndex::clearPos(){
    positional_index.clear();
    positional_size = 0;
}

void DynamicIndex::clearNonPos(){
    nonpositional_index.clear();
    nonpositional_size = 0;
}