#include <iostream>

#include "doc_analyzer/Tests/blocktester.h"

using namespace std;

int main(int argc, char **argv) {
    Tests::BlockTester bt;
    bt.test();

    cout << "All tests passed successfully" << endl;
    
    return 0;
}
