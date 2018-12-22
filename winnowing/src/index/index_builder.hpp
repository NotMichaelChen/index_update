#ifndef INDEX_BUILDER_H
#define INDEX_BUILDER_H

#include "doc_partitioner/partitioner.hpp"
#include "document_store.hpp"
#include "document_version_table.hpp"
#include "global_types.hpp"
#include "lexicon.hpp"
#include "dynamic_index.hpp"
#include "static_index.hpp"

#include <string>

class IndexBuilder {
private:
    // DocumentStore docstore;
    Lexicon lex;
    DynamicIndex dynamicindex;
    StaticIndex staticwriter;

    DocumentVersionTable dvtable;
    Partitioner winnower;

    std::unordered_map<std::string, FragID> fragmenthashtable;
    std::unordered_map<std::string, size_t> pagetable;

    void insertFragment(const Fragment& currfrag);

    size_t currdid;
    std::string working_dir;
public:
    IndexBuilder(const std::string& directory, const unsigned int b, const unsigned int w);

    // Partition the page by winnowing and index the fragments
    void insertDocument(const std::string& pageurl, const std::string& pagecontent);

};

#endif
