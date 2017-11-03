#include "blocktester.h"

#include <assert.h>
#include <vector>
#include <memory>

#include "../Matcher/stringencoder.h"
#include "../Matcher/stringencoder.h"

using namespace std;

namespace Tests {
    void BlockTester::test() {
		testgetCommonBlocks();
    }
    
    void BlockTester::testgetCommonBlocks() {
        assert(getCommonBlocksSize("", "", 5) == 0);
        assert(getCommonBlocksSize("", "a b c", 5) == 0);
        assert(getCommonBlocksSize("cde", "", 5) == 0);
        assert(getCommonBlocksSize("a b c d", "a b c d", 2) == 3);
        assert(getCommonBlocksSize("a b b c d c b", "a b b d c", 2) == 3);
        assert(getCommonBlocksSize("a b b e f d e g r", "a b b e g r", 2) == 5);
        assert(getCommonBlocksSize("a c d e", "a c d e", 10) == 0);
	}
    
    void BlockTester::testextendBlocks() {
		vector<shared_ptr<Matcher::Block>> testvec;
		
		extendBlocksTest(testvec, "", "", 5);
		assert(testvec.size() == 0);
		testvec.clear();
		
		extendBlocksTest(testvec, "", "a b c", 5);
		assert(testvec.size() == 0);
		testvec.clear();
		
		extendBlocksTest(testvec, "cde", "", 5);
		assert(testvec.size() == 0);
		testvec.clear();
		
		extendBlocksTest(testvec, "a b c d", "a b c d", 2);
		assert(testvec.size() == 1);
		testvec.clear();
		
		extendBlocksTest(testvec, "a b b c d c b", "a b b d c", 2);
		assert(testvec.size() == 2);
		testvec.clear();
		
		extendBlocksTest(testvec, "a b b e f d e g r", "a b b e g r", 2);
		assert(testvec.size() == 2);
		assert(testvec[0]->run.size() == 4);
		assert(testvec[1]->run.size() == 3);
		testvec.clear();
		
		extendBlocksTest(testvec, "a c d e", "a c d e", 10);
		assert(testvec.size() == 0);
		testvec.clear();
	}

    //Test returns size of input
    size_t BlockTester::getCommonBlocksSize(string a, string b, int blocksize) {
        Matcher::StringEncoder se(a, b);
        auto result = Matcher::getCommonBlocks(blocksize, se);
        return result.size();
    }
    
    //Simulates running getCommonBlocks then extendBlocks
    void BlockTester::extendBlocksTest(vector<shared_ptr<Matcher::Block>>& allblocks, string a, string b, int blocksize) {
		Matcher::StringEncoder se(a, b);
        auto result = Matcher::getCommonBlocks(blocksize, se);
        Matcher::extendBlocks(allblocks, se);
	}
}
