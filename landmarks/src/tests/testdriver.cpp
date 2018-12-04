#include "testdriver.hpp"

#include "tests/doc_parsing/difftest.hpp"

// All tests should:
// - Begin by printing which function is running

namespace tester
{

void testdriver() {
    test_diff();
    test_makeDiffRange();
}

}