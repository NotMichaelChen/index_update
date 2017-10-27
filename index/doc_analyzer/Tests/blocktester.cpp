#include "blocktester.h"

#include <assert.h>

#include "../Matcher/stringencoder.h"
#include "../Matcher/stringencoder.h"

namespace Tests {
    void blockTester::test() {
        assert(getCommonBlocksSize("", "", 5) == 0);
        assert(getCommonBlocksSize("", "a b c", 5) == 0);
        assert(getCommonBlocksSize("cde", "", 5) == 0);
        assert(getCommonBlocksSize("a b c d", "a b c d", 2) == 3);
        assert(getCommonBlocksSize("a b b c d c b", "a b b d c", 2) == 3);
        assert(getCommonBlocksSize("a c d e", "a c d e", 10) == 0);
    }

    //Test returns size of input
    size_t blockTester::getCommonBlocksSize(std::string a, std::string b, int blocksize) {
        Matcher::StringEncoder se(a, b);
        auto result = Matcher::getCommonBlocks(blocksize, se);
        return result.size();
    }
}
