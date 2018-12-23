#include "document_version_table.hpp"

#include <iostream>
#include <vector>

DocumentVersionTable::VersionInfo::VersionInfo(const std::vector<Fragment>& f) : fragments(f) {}

size_t DocumentVersionTable::addDocVersion(const std::vector<Fragment>& fragments) {
    size_t did = this->dvtable.size();
    DVTEntry newEntry;
    newEntry.versions.push_back(VersionInfo(fragments));
    this->dvtable.push_back(newEntry);
    return did;
}

size_t DocumentVersionTable::updateDocVersion(size_t docid, const std::vector<Fragment>& fragments) {
    if (docid < this->dvtable.size()) {
        this->dvtable[docid].versions.push_back(VersionInfo(fragments));
    }
    return this->dvtable[docid].versions.size();
}

size_t DocumentVersionTable::size() const {
    return dvtable.size();
}

void DocumentVersionTable::displayStat() const {
    size_t totalvers = 0;
    size_t minvercnt = (size_t) - 1;
    size_t maxvercnt = 0;

    size_t totalfragcount = 0;
    size_t totalfragsize = 0;
    size_t minfragize = (size_t) - 1;
    size_t maxfragize = 0;

    for (auto e : this->dvtable) {
        totalvers += e.versions.size();
        minvercnt = std::min(minvercnt, e.versions.size());
        maxvercnt = std::min(maxvercnt, e.versions.size());
        for (auto vi : e.versions) {
            totalfragcount += vi.fragments.size();
            for (auto f : vi.fragments) {
                totalfragsize += f.fragcontent.size();
                minfragize = std::min(minfragize, f.fragcontent.size());
                maxfragize = std::max(maxfragize, f.fragcontent.size());
            }
        }
    }
    std::cout << "DocumentVersionTable: " << this->dvtable.size() << " documents\n";
    std::cout << "Per document\n\tavg # of versions: " << (float)totalvers/(float)this->dvtable.size()
        << "\n\tmin # of versions: " << minvercnt << "\n\tmax # of versions: " << maxvercnt << '\n';
    std::cout << "Across versions\n\tTotal # of fragments: " << totalfragcount << "\n\tavg frag size: " << (float)totalfragsize/(float)totalfragcount
        << "\n\tmin frag size: " << minfragize << "\n\tmax frag size: "<< maxfragize << '\n';
}