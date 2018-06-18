#include "libs/catch.hpp"

#include "doc_analyzer/Matcher/blockmatching.hpp"
#include "doc_analyzer/Matcher/graph.h"

Matcher::BlockGraph makeGraph(std::string a, std::string b, int blocksize) {
    Matcher::StringEncoder se(a, b);
    std::vector<Matcher::Block> allblocks = Matcher::getCommonBlocks(blocksize, se);
    Matcher::resolveIntersections(allblocks);
    return Matcher::BlockGraph(allblocks);
}

TEST_CASE("Test block graph", "[blockgraph]") {
    auto G = makeGraph("a b c d e f g h i j k l", "a b f g x y z h i j l k", 2);
    std::vector<Matcher::Block> v = G.getAllVertices();

    REQUIRE(v.size() == 3);
}