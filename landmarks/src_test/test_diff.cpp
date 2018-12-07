#include "libs/catch.hpp"

#include <iostream>
#include <vector>

#include "doc_parsing/diff.hpp"

// Helper Functions

bool diffentryEqual(DiffEntry& entry, bool i, unsigned int o, unsigned int n) {
    return entry.isIns == i
        && entry.oldpos == o
        && entry.newpos == n;
}

// Testing Functions

TEST_CASE("Test diff", "[diff]") {
    std::vector<int> vec1, vec2;
    std::vector<DiffEntry> res;

    vec1 = {1, 2, 3, 4, 5};
    vec2 = {1, 7, 8, 9, 5};
    res = diff(vec1, vec2);

    REQUIRE(res.size() == 6);
    REQUIRE(res[0].isIns == false); REQUIRE(res[0].oldpos == 1);
    REQUIRE(res[1].isIns == false); REQUIRE(res[1].oldpos == 2);
    REQUIRE(res[2].isIns == false); REQUIRE(res[2].oldpos == 3);
    REQUIRE(res[3].isIns == true); REQUIRE(res[3].oldpos == 4); REQUIRE(res[3].newpos == 1);
    REQUIRE(res[4].isIns == true); REQUIRE(res[4].oldpos == 4); REQUIRE(res[4].newpos == 2);
    REQUIRE(res[5].isIns == true); REQUIRE(res[5].oldpos == 4); REQUIRE(res[5].newpos == 3);

    vec1.clear();
    vec2.clear();
    vec1 = {1, 2, 3, 4, 5};
    vec2 = {6, 2, 7, 4, 8};
    res = diff(vec1, vec2);

    REQUIRE(res.size() == 6);
    REQUIRE(res[0].isIns == false); REQUIRE(res[0].oldpos == 0);
    REQUIRE(res[1].isIns == true); REQUIRE(res[1].oldpos == 1); REQUIRE(res[1].newpos == 0);
    REQUIRE(res[2].isIns == false); REQUIRE(res[2].oldpos == 2);
    REQUIRE(res[3].isIns == true); REQUIRE(res[3].oldpos == 3); REQUIRE(res[3].newpos == 2);
    REQUIRE(res[4].isIns == true); REQUIRE(res[4].oldpos == 4); REQUIRE(res[4].newpos == 4);
    REQUIRE(res[5].isIns == false); REQUIRE(res[5].oldpos == 4);
}

TEST_CASE("Test makeDiffRange", "[diff]") {
    std::cout << "Testing makeDiffRange..." << std::endl;

    std::vector<int> vec1, vec2;
    std::vector<DiffRange> res;

    vec1 = {1, 2, 3, 4, 5};
    vec2 = {1, 7, 8, 9, 5};
    res = makeDiffRange(vec1, vec2);

    REQUIRE(res.size() == 2);

    vec1.clear();
    vec2.clear();
    vec1 = {1, 2, 3, 4, 5};
    vec2 = {6, 2, 7, 4, 8};
    res = makeDiffRange(vec1, vec2);

    REQUIRE(res.size() == 6);
}