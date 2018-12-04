#include <iostream>

#include "doc_parsing/diff.hpp"
#include "tests/testdriver.hpp"

int main(int argc, char const *argv[])
{
    if(argc >= 2) {
        std::string secondarg = argv[1];
        if(secondarg == "test") {
            tester::testdriver();
            return 0;
        }
    }

    std::cout << "Hello World!" << std::endl;
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {1, 7, 8, 9, 5};

    auto res = diff(a, b);
    for(auto i : res)
        std::cout << i.isIns << ' ' << i.oldpos << ' ' << i.newpos << std::endl;

    return 0;
}
