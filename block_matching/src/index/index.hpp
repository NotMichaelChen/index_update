#ifndef INDEX_HPP
#define INDEX_HPP

#include <vector>
#include <unordered_map>

#include "libs/sparsepp/spp.h"
#include "lexicon.hpp"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"
#include "static_index.hpp"
#include "global_parameters.hpp"
#include "doc_analyzer/analyzer.h"
#include "posting.hpp"
#include "dynamic_index.hpp"

//This index does not use compression
class Index {
public:
    //Directory is simply a name that the index will save all of its files under
    Index(std::string directory);
    void insert_document(std::string& url, std::string& newpage);
    //Temporary return type: returns docIDs for now
    std::vector<unsigned int> query(std::vector<std::string> words);

    void dump();
    void restore();
    void clear();

    void printSize();

private:
    void insertNPPostings(MatcherInfo& results);
    void insertPPostings(MatcherInfo& results);

    //Data structures
    //Note: Posting lists are *lazily sorted*, that is, docIDs are stored randomly until they need to be sorted.
    //Indexes on disk are guaranteed to be sorted (due to delta compression)

    std::string working_dir;

    DocumentStore docstore;
    TranslationTable transtable;
    Lexicon lex;
    DynamicIndex dynamicindex;
    StaticIndex staticwriter;
};

#endif
