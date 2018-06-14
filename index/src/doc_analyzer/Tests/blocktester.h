#ifndef BLOCKTESTER_H
#define BLOCKTESTER_H

#include <vector>
#include <memory>

#include "../Matcher/block.h"

namespace Tests {
    class BlockTester {
    public:
        void test();
    private:
		void testgetCommonBlocks();
		void testextendBlocks();
		void testresolveIntersections();
		void extendBlocksTest(std::vector<std::shared_ptr<Matcher::Block>>& allblocks, std::string a, std::string b, int blocksize);
        size_t getCommonBlocksSize(std::string a, std::string b, int blocksize);
        void resolveIntersectionsTest(std::vector<std::shared_ptr<Matcher::Block>>& allblocks, std::string a, std::string b, int blocksize);
    };
}

#endif
