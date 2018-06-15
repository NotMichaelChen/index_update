#include "libs/catch.hpp"

#include <iostream>

#include "doc_analyzer/Matcher/matcher.h"

TEST_CASE("Test getPostings", "[matcher]") {
    std::string a = "a b c d e f g h i j k l";
    std::string b = "a b f g x y z h i j l k";
    
    Matcher::StringEncoder se(a, b);

    auto r1 = Matcher::getOptimalBlocks(se, 2, 20, 0);
    unsigned int fragID = 0;
    auto results = Matcher::getPostings(r1, 0, fragID, se);

    REQUIRE(results.first.size() == 8);
    REQUIRE(results.second.size() == 5);
    REQUIRE(fragID == 2);
}