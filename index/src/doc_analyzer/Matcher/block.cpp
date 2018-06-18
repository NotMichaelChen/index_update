#include "block.h"

#include <iostream>

using namespace std;

namespace Matcher {

    Block::Block() : oldloc(-1), newloc(-1) {}
    Block::Block(int o, int n, vector<int> b) : run(b), oldloc(o), newloc(n)  {}
    
    int Block::oldendloc() { return oldloc + run.size() - 1; }
    int Block::newendloc() { return newloc + run.size() - 1; }

    bool Block::isValid() { return oldloc >= 0 && newloc >= 0 && !run.empty(); }

    ostream& operator<<(ostream& os, const Block& bl) {
        os << bl.oldloc << "-" << bl.newloc << "-" << bl.run.size();
        return os;
    }

    bool operator==(const Block& lhs, const Block& rhs) {
        return lhs.oldloc == rhs.oldloc && lhs.newloc == rhs.newloc && lhs.run.size() == rhs.run.size();
    }
    
    bool compareOld(const Block& lhs, const Block& rhs) {
        return lhs.oldloc < rhs.oldloc;
    }
    bool compareNew(const Block& lhs, const Block& rhs) {
        return lhs.newloc < rhs.newloc;
    }
    bool compareSizeGreater(const Block& lhs, const Block& rhs) {
        return lhs.run.size() > rhs.run.size();
    }
}
