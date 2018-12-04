#pragma once

#include <string>
#include <vector>
#include <map>

#include "posting.hpp"
#include "redis_interfaces/documentstore.hpp"
#include "lexicon.hpp"
#include "landmark/landmarkdirectory.hpp"
#include "doc_parsing/docparser.hpp"

class Index {
public:
    //Directory is simply a name that the index will save all of its files under
    Index(std::string directory);
    void insert_document(std::string& url, std::string& newpage);

private:
    void insertNPPostings(DocumentParser& results);
    void insertPPostings(DocumentParser& results);

    //Data structures
    //Note: Posting lists are *lazily sorted*, that is, docIDs are stored randomly until they need to be sorted.
    //Indexes on disk are guaranteed to be sorted (due to delta compression)
    std::map<unsigned int, std::vector<Posting>> positional_index;
    std::map<unsigned int, std::vector<nPosting>> nonpositional_index;

    unsigned long positional_size;
    unsigned long nonpositional_size;

    DocumentStore docstore;
    Lexicon lex;
    LandmarkDirectory landdir;
};
