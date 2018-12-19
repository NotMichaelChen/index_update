#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "document_readers/WETreader.hpp"
#include "utility/morph.hpp"
#include "doc_partitioner/partitioner.hpp"
#include "index/index_builder.hpp"

template<typename T>
void printVec(const std::vector<T>& v) {
    for (const T& e : v) {
        std::cout << e << "\n\n";
    }
    std::cout << '\n';
}

int main() {
    IndexBuilder indexer(50, 100);

    WETReader wr("wet_files");
    int numdocstoread = 0;
    std::string lastdoc = wr.getCurrentDocument();
    std::string lasturl = wr.getURL();
    wr.nextDocument();
    while (numdocstoread++ < 1 && wr.isValid()) {
        std::string currdoc = wr.getCurrentDocument();
        std::string currurl = wr.getURL();
        DocumentMorpher dm(lastdoc, currdoc, 100);
        std::cout << "==========\n" << currurl << '\n';
        while (dm.isValid()) {
            indexer.insertDocument(currurl, dm.getDocument());
            dm.nextVersion();
        }
        lastdoc = currdoc;
        lasturl = currurl;
        wr.nextDocument();
    }

    // std::cout << vs[0] << '\n';

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
