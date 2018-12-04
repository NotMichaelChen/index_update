#include "libs/catch.hpp"

#include "doc_analyzer/Matcher/blockmatching.hpp"

//Test returns size of input
size_t getCommonBlocksSize(std::string a, std::string b, int blocksize) {
    StringEncoder se(a, b);
    auto result = getCommonBlocks(blocksize, se);
    return result.size();
}

std::vector<std::shared_ptr<Block>> getCommonBlocksTest(std::string a, std::string b, int blocksize) {
    StringEncoder se(a, b);
    auto result = getCommonBlocks(blocksize, se);
    return result;
}

void extendBlocksTest(std::vector<std::shared_ptr<Block>>& allblocks, std::string a, std::string b, int blocksize) {
    StringEncoder se(a, b);
    allblocks = getCommonBlocks(blocksize, se);
    extendBlocks(allblocks, se);
}

//Simulates running all three functions to produce a final list of blocks
void resolveIntersectionsTest(std::vector<std::shared_ptr<Block>>& allblocks, std::string a, std::string b, int blocksize) {
    StringEncoder se(a, b);
    allblocks = getCommonBlocks(blocksize, se);
    extendBlocks(allblocks, se);
    resolveIntersections(allblocks);
}

TEST_CASE("Test getCommonBlocks", "[block]") {
    REQUIRE(getCommonBlocksSize("", "", 0) == 0);
    REQUIRE(getCommonBlocksSize("", "", -1) == 0);
    REQUIRE(getCommonBlocksSize("", "", 5) == 0);
    REQUIRE(getCommonBlocksSize("", "a b c", 5) == 0);
    REQUIRE(getCommonBlocksSize("cde", "", 5) == 0);
    REQUIRE(getCommonBlocksSize("a b c d", "a b c d", 2) == 3);
    REQUIRE(getCommonBlocksSize("a b b c d c b", "a b b d c", 2) == 3);
    REQUIRE(getCommonBlocksSize("a b b e f d e g r", "a b b e g r", 2) == 5);
    REQUIRE(getCommonBlocksSize("a b", "a b a b a b", 2) == 3);
    REQUIRE(getCommonBlocksSize("a c d e", "a c d e", 10) == 0);
    REQUIRE(getCommonBlocksSize("a b c d e f g", "a b i a b c j c d e k c d e f g", 2) == 9);
    REQUIRE(getCommonBlocksSize("a b c d e a b", "a b c f c d e g a b", 2) == 7);
}

TEST_CASE("Test extendBlocks", "[block]") {
    std::vector<std::shared_ptr<Block>> testvec;

    extendBlocksTest(testvec, "", "", 0);
    REQUIRE(testvec.size() == 0);
    testvec.clear();

    extendBlocksTest(testvec, "", "", -1);
    REQUIRE(testvec.size() == 0);
    testvec.clear();
    
    extendBlocksTest(testvec, "", "", 5);
    REQUIRE(testvec.size() == 0);
    testvec.clear();
    
    extendBlocksTest(testvec, "", "a b c", 2);
    REQUIRE(testvec.size() == 0);
    testvec.clear();
    
    extendBlocksTest(testvec, "cde", "", 1);
    REQUIRE(testvec.size() == 0);
    testvec.clear();
    
    extendBlocksTest(testvec, "a b c d", "a b c d", 2);
    REQUIRE(testvec.size() == 1);
    testvec.clear();
    
    extendBlocksTest(testvec, "a b b c d c b", "a b b d c", 2);
    REQUIRE(testvec.size() == 2);
    testvec.clear();
    extendBlocksTest(testvec, "a b b d c", "a b b c d c b", 2);
    REQUIRE(testvec.size() == 2);
    testvec.clear();

    extendBlocksTest(testvec, "a b b e f d e g r", "a b b e g r", 2);
    REQUIRE(testvec.size() == 2);
    REQUIRE(testvec[0]->len == 4);
    REQUIRE(testvec[1]->len == 3);
    testvec.clear();

    extendBlocksTest(testvec, "a b", "a b a b a b", 2);
    REQUIRE(testvec.size() == 3);
    testvec.clear();
    extendBlocksTest(testvec, "a b a b a b", "a b", 2);
    REQUIRE(testvec.size() == 3);
    testvec.clear();
    
    extendBlocksTest(testvec, "a c d e", "a c d e", 10);
    REQUIRE(testvec.size() == 0);
    testvec.clear();

    extendBlocksTest(testvec, "a b c d e f g", "a b i a b c j c d e k c d e f g", 2);
    REQUIRE(testvec.size() == 4);
    testvec.clear();
    extendBlocksTest(testvec, "a b i a b c j c d e k c d e f g", "a b c d e f g", 2);
    REQUIRE(testvec.size() == 4);
    testvec.clear();

    extendBlocksTest(testvec, "a b c d e a b", "a b c f c d e g a b", 2);
    REQUIRE(testvec.size() == 5);
    testvec.clear();
    extendBlocksTest(testvec, "a b c f c d e g a b", "a b c d e a b", 2);
    REQUIRE(testvec.size() == 5);
    testvec.clear();

    extendBlocksTest(testvec, "a b c d e f g", "a b c d z c d e f g y e f g", 2);
    REQUIRE(testvec.size() == 3);
    testvec.clear();
    extendBlocksTest(testvec, "a b c d z c d e f g y e f g", "a b c d e f g", 2);
    REQUIRE(testvec.size() == 3);
    testvec.clear();
    
    extendBlocksTest(testvec, "a b a b e a b a b", "a b a b a b a b", 2);
    REQUIRE(testvec.size() == 6);
    testvec.clear();
}

TEST_CASE("Test resolveIntersections", "[block]") {
    std::vector<std::shared_ptr<Block>> testvec;
    
    resolveIntersectionsTest(testvec, "", "", 2);
    REQUIRE(testvec.size() == 0);
    testvec.clear();
    
    resolveIntersectionsTest(testvec, "", "a b c", 5);
    REQUIRE(testvec.size() == 0);
    testvec.clear();
    
    resolveIntersectionsTest(testvec, "cde", "", 5);
    REQUIRE(testvec.size() == 0);
    testvec.clear();
    
    resolveIntersectionsTest(testvec, "a b c d", "a b c d", 2);
    REQUIRE(testvec.size() == 1);
    testvec.clear();

    resolveIntersectionsTest(testvec, "a b", "a b a b a b", 2);
    REQUIRE(testvec.size() == 3);
    testvec.clear();
    
    resolveIntersectionsTest(testvec, "a b c d e f g", "a b i a b c j c d e k c d e f g", 2);
    REQUIRE(testvec.size() == 5);
    testvec.clear();
    
    resolveIntersectionsTest(testvec, "a b c", "a b c", 10);
    REQUIRE(testvec.size() == 0);
    testvec.clear();

    resolveIntersectionsTest(testvec, "a b c d e a b", "a b c f c d e g a b", 2);
    REQUIRE(testvec.size() == 6);
    testvec.clear();
}