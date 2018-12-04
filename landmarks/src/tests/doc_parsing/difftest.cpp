#include "difftest.hpp"

#include <cassert>
#include <iostream>
#include <vector>

#include "doc_parsing/diff.hpp"

namespace tester
{

// Helper Functions

bool diffentryEqual(DiffEntry& entry, bool i, unsigned int o, unsigned int n) {
    return entry.isIns == i
        && entry.oldpos == o
        && entry.newpos == n;
}

// Testing Functions

void test_diff() {
    std::cout << "Testing diff..." << std::endl;

    std::vector<int> vec1, vec2;
    std::vector<DiffEntry> res;

    vec1 = {1, 2, 3, 4, 5};
    vec2 = {1, 7, 8, 9, 5};
    res = diff(vec1, vec2);

    assert(res.size() == 6);
    assert(res[0].isIns == false && res[0].oldpos == 1);
    assert(res[1].isIns == false && res[1].oldpos == 2);
    assert(res[2].isIns == false && res[2].oldpos == 3);
    assert(res[3].isIns == true && res[3].oldpos == 4 && res[3].newpos == 1);
    assert(res[4].isIns == true && res[4].oldpos == 4 && res[4].newpos == 2);
    assert(res[5].isIns == true && res[5].oldpos == 4 && res[5].newpos == 3);

    vec1.clear();
    vec2.clear();
    vec1 = {1, 2, 3, 4, 5};
    vec2 = {6, 2, 7, 4, 8};
    res = diff(vec1, vec2);

    assert(res.size() == 6);
    assert(res[0].isIns == false && res[0].oldpos == 0);
    assert(res[1].isIns == true && res[1].oldpos == 1 && res[1].newpos == 0);
    assert(res[2].isIns == false && res[2].oldpos == 2);
    assert(res[3].isIns == true && res[3].oldpos == 3 && res[3].newpos == 2);
    assert(res[4].isIns == true && res[4].oldpos == 4 && res[4].newpos == 4);
    assert(res[5].isIns == false && res[5].oldpos == 4);
}

void test_makeDiffRange() {
    std::cout << "Testing makeDiffRange..." << std::endl;

    std::vector<int> vec1, vec2;
    std::vector<DiffRange> res;

    vec1 = {1, 2, 3, 4, 5};
    vec2 = {1, 7, 8, 9, 5};
    res = makeDiffRange(vec1, vec2);

    assert(res.size() == 2);

    vec1.clear();
    vec2.clear();
    vec1 = {1, 2, 3, 4, 5};
    vec2 = {6, 2, 7, 4, 8};
    res = makeDiffRange(vec1, vec2);

    assert(res.size() == 6);
}

}