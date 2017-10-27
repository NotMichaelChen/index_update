#ifndef INDEX_HPP
#define INDEX_HPP

#include <vector>
#include <map>

#include "lexicon.hpp"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

//This index does not use compression
class Index {
public:
    void Index();
    void insert_document(std::string& url, std::string& newpage);
private:
    std::map<std::string, std::vector<Posting>> positional_index;
    std::map<std::string, std::vector<nPosting>> nonpositional_index;

    Lexicon lex;
    Structures::DocumentStore docstore;
    Structures::TranslationTable transtable;
};

#endif
