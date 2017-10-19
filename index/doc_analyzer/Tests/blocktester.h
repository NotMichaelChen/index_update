#ifndef BLOCKTESTER_H
#define BLOCKTESTER_H

#include "../Matcher/block.h"

namespace Tests {
    class blockTester {
    public:
        void test();
    private:
        size_t getCommonBlocksSize(std::string a, std::string b, int blocksize);
    };
}

#endif
