#include "libs/catch.hpp"

#include "doc_analyzer/Matcher/blockmatching.hpp"
#include "doc_analyzer/Matcher/graph.h"

BlockGraph makeGraph(std::string a, std::string b, int blocksize) {
    StringEncoder se(a, b);
    std::vector<std::shared_ptr<Block>> allblocks = getCommonBlocks(blocksize, se);
    resolveIntersections(allblocks);
    return BlockGraph(allblocks);
}

TEST_CASE("Test block graph", "[blockgraph]") {
    auto G = makeGraph("a b c d e f g h i j k l", "a b f g x y z h i j l k", 2);
    std::vector<Block> v = G.getAllVertices();

    REQUIRE(v.size() == 3);
}