#include "index_builder.hpp"
#include "doc_partitioner/partitioner.hpp"

#include <string>
#include <vector>
#include <utility>

IndexBuilder::IndexBuilder(const unsigned int b, const unsigned int w) : winnower(b, w), currdid(0) {}

void IndexBuilder::insertDocument(const std::string& pageurl, const std::string& pagecontent) {
    std::vector<Fragment> fragments = winnower.partitionPage(currdid, pagecontent);
    std::cout << "Partition finished: " << fragments.size() << " fragments\n";

    if (this->pagetable.find(pageurl) != this->pagetable.end()) {
        std::cout << "Updating version of " << pageurl << '\n';
        if (!this->dvtable.updateDocVersion(this->pagetable.at(pageurl), fragments)) {
            std::cerr << "Failed updateDocVersion\n";
        }
    } else {
        size_t newdid = this->dvtable.addDocVersion(fragments);
        this->pagetable.insert(std::make_pair(pageurl, newdid));
    }
    std::cout << dvtable.size() << ' ' << pagetable.size() << '\n';

    currdid++;
}