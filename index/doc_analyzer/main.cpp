#include <iostream>
#include <fstream>
#include <getopt.h>
#include <vector>
#include <algorithm>
#include <memory>

#include "Matcher/matcher.h"
#include "Matcher/stringencoder.h"
#include "Matcher/block.h"
#include "Matcher/graph.h"
#include "Matcher/distancetable.h"
#include "Matcher/translate.h"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"
#include "Tests/blocktester.h"

using namespace std;

int main(int argc, char **argv) {
    Tests::BlockTester bt;
    bt.test();

    cout << "All tests passed successfully" << endl;
    
    return 0;
}
