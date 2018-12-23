#ifndef DOCUMENT_VERSION_TABLE_H
#define DOCUMENT_VERSION_TABLE_H

#include "global_types.hpp"

#include <vector>

class DocumentVersionTable {
private:
    struct VersionInfo {
        VersionInfo(const std::vector<Fragment>& f);
        std::vector<Fragment> fragments;
    };

    struct DVTEntry {
        std::vector<VersionInfo> versions;
    };

    std::vector<DVTEntry> dvtable;

public:
    // Add new page with the returned docid. Caller must check if the new docid in DVT matches with PageTable
    size_t addDocVersion(const std::vector<Fragment>& fragments);

    size_t updateDocVersion(size_t docid, const std::vector<Fragment>& fragments);

    size_t size() const;

    void displayStat() const;
};

#endif
