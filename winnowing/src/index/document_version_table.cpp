#include "document_version_table.hpp"

#include <vector>

DocumentVersionTable::VersionInfo::VersionInfo(const std::vector<Fragment>& f) : fragments(f) {}

size_t DocumentVersionTable::addDocVersion(const std::vector<Fragment>& fragments) {
    size_t did = this->dvtable.size();
    DVTEntry newEntry;
    newEntry.versions.push_back(VersionInfo(fragments));
    this->dvtable.push_back(newEntry);
    return did;
}

bool DocumentVersionTable::updateDocVersion(size_t docid, const std::vector<Fragment>& fragments) {
    if (docid >= this->dvtable.size()) { return false; }

    this->dvtable[docid].versions.push_back(VersionInfo(fragments));
    return true;
}

size_t DocumentVersionTable::size() const {
    return dvtable.size();
}