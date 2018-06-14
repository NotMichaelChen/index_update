#include "libs/catch.hpp"

#include "doc_analyzer/Matcher/stringencoder.h"

TEST_CASE("Test stringencoder", "[stringencoder]") {
    Matcher::StringEncoder se("a b c d", "c b a e");

        std::vector<unsigned int> oldstream(se.getOldIter(), se.getOldEnd());
        std::vector<unsigned int> newstream(se.getNewIter(), se.getNewEnd());

        std::vector<std::string> oldwords = se.decodeStream(oldstream);
        std::vector<std::string> newwords = se.decodeStream(newstream);

        REQUIRE(oldwords.size() == 4);
        REQUIRE(newwords.size() == 4);

        REQUIRE(oldwords[0] == "a");
        REQUIRE(oldwords[1] == "b");
        REQUIRE(oldwords[2] == "c");
        REQUIRE(oldwords[3] == "d");

        REQUIRE(newwords[0] == "c");
        REQUIRE(newwords[1] == "b");
        REQUIRE(newwords[2] == "a");
        REQUIRE(newwords[3] == "e");

    std::string s = "a";
    REQUIRE(se.getNewCount(s) == 1);
    s = "";
    REQUIRE(se.getNewCount(s) == 0);

    s = "e";
    REQUIRE(se.inNew(s));
    s = "d";
    REQUIRE(!se.inNew(s));
    s = "d";
    REQUIRE(se.inOld(s));
    s = "sdifjoaeariog reg r ";
    REQUIRE(!se.inOld(s));
}