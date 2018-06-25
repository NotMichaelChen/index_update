#include "block.h"

#include <iostream>

using namespace std;

Block::Block() : oldloc(-1), newloc(-1), len(0) {}
Block::Block(int o, int n, size_t l) : oldloc(o), newloc(n), len(l) {}

int Block::oldendloc() { return oldloc + len - 1; }
int Block::newendloc() { return newloc + len - 1; }

ostream& operator<<(ostream& os, const Block& bl) {
    os << bl.oldloc << "-" << bl.newloc << "-" << bl.len;
    return os;
}

bool operator==(const shared_ptr<Block>& lhs, const shared_ptr<Block>& rhs) {
    return lhs->oldloc == rhs->oldloc && lhs->newloc == rhs->newloc && lhs->len == rhs->len;
}

bool compareOld(const shared_ptr<Block>& lhs, const shared_ptr<Block>& rhs) {
    return lhs->oldloc < rhs->oldloc;
}
bool compareNew(const shared_ptr<Block>& lhs, const shared_ptr<Block>& rhs) {
    return lhs->newloc < rhs->newloc;
}
bool compareSizeGreater(const shared_ptr<Block>& lhs, const shared_ptr<Block>& rhs) {
    return lhs->len > rhs->len;
}

bool compareStrict(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs) {
    if(lhs->oldloc != rhs->oldloc)
        return lhs->oldloc < rhs->oldloc;
    else if(lhs->newloc != rhs->newloc)
        return lhs->newloc < rhs->newloc;
    else if(lhs->len != rhs->len)
        return lhs->len < rhs->len;
    else
        return false;
}