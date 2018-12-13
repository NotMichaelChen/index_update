#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "document_readers/WETreader.hpp"
#include "utility/morph.hpp"
#include "doc_partitioner/partitioner.hpp"

template<typename T>
void printVec(const std::vector<T>& v) {
    for (const T& e : v) {
        std::cout << e << "\n\n";
    }
    std::cout << '\n';
}

int main(int argc, char **argv) {
    WETReader wr("wet_files");
    std::vector<std::string> vs;
    int n = 1;
    while (n-- && wr.isValid()) {
        vs.push_back(wr.getCurrentDocument());
        wr.nextDocument();
    }

    // std::cout << vs[0] << '\n';

    Partitioner docpart(50, 100);
    std::vector<std::string> partitioned = docpart.partitionPage(vs[0]);
    printVec(partitioned);

    // DocumentMorpher dm(vs[0], vs[1], 100);
    // while (dm.isValid()) {
    //     dm.nextVersion();
    //     vs.push_back(dm.getDocument());
    // }

    // // for (auto d : vs) {
    // //     cout << d << endl;
    // //     cout << "==============" << endl;
    // // }
    // cout << vs.size() << endl;

    // vector<size_t> testHashedFile = {23, 17, 45, 13, 48, 13, 48, 87, 19, 7, 21, 12, 29, 13};
    // vector<size_t> cuts = cutFile(testHashedFile);
    // vector<size_t> expectedCuts = {0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0};
    // assert(cuts == expectedCuts);



    return 0;
}
