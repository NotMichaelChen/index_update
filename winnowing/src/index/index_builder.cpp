#include "index_builder.hpp"
#include "doc_partitioner/partitioner.hpp"
#include "utility/md5.h"

#include <string>
#include <vector>
#include <utility>

IndexBuilder::IndexBuilder(const unsigned int b, const unsigned int w) : winnower(b, w), currdid(0) {}

void IndexBuilder::insertDocument(const std::string& pageurl, const std::string& pagecontent) {
    std::vector<Fragment> fragments = winnower.partitionPage(currdid, pagecontent);
    std::cout << "Partition finished: " << fragments.size() << " fragments\n";

    if (this->pagetable.find(pageurl) != this->pagetable.end()) {
        size_t newver = this->dvtable.updateDocVersion(this->pagetable.at(pageurl), fragments);
        std::cout << "New version: "<< newver << '\n';
    } else {
        size_t newdid = this->dvtable.addDocVersion(fragments);
        this->pagetable.insert(std::make_pair(pageurl, newdid));
        currdid++;
    }
    std::cout << dvtable.size() << ' ' << pagetable.size() << '\n';

    for (const Fragment& currfrag : fragments) {
        std::string md5frag = md5(currfrag.fragcontent);
        FragID currfragid = std::make_pair(currfrag.docid, currfrag.startpos);
        if (fragmenthashtable.find(md5frag) != fragmenthashtable.end()) {
            std::cout << "Duplicate frag found! (" << currfrag.docid << ' ' << currfrag.startpos << ")\n";
            continue;
        }
        fragmenthashtable.insert(std::make_pair(md5frag, currfragid));
    }
}