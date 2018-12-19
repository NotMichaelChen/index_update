#ifndef INDEX_BUILDER_H
#define INDEX_BUILDER_H

#include "doc_partitioner/partitioner.hpp"
#include "document_store.hpp"
#include "document_version_table.hpp"
#include "global_types.hpp"

#include <string>

class IndexBuilder {
private:
    // DocumentStore docstore;
    PageTable pagetable;
    // Lexicon lex;
    // DynamicIndex dynamicindex;
    // StaticIndex staticwriter;

    // FragmentHashTable
    DocumentVersionTable dvtable;

    Partitioner winnower;

    // insertFragment

    size_t currdid;
public:
    IndexBuilder(const unsigned int b, const unsigned int w);

    // Partition the page by winnowing and index the fragments
    void insertDocument(const std::string& pageurl, const std::string& pagecontent);

};

#endif
