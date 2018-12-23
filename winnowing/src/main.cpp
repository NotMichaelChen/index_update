#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "document_readers/WETreader.hpp"
#include "utility/morph.hpp"
#include "doc_partitioner/partitioner.hpp"
#include "index/index_builder.hpp"
#include "utility/timer.hpp"

template<typename T>
void printVec(const std::vector<T>& v) {
    for (const T& e : v) {
        std::cout << e << "\n\n";
    }
    std::cout << '\n';
}

int main() {
    Utility::Timer maintimer;
    maintimer.start();
    IndexBuilder indexer("", 50, 100);
    WETReader wr("wet_files");
    std::string lastdoc = wr.getCurrentDocument();
    std::string lasturl = wr.getURL();
    wr.nextDocument();
    size_t i = 1;
    while (i-- && wr.isValid()) {
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
    maintimer.stop();
    std::cout << "\n\nTotal indexing time: " << maintimer.getCumulative() << '\n';
    indexer.displayStat();
    return 0;
}
