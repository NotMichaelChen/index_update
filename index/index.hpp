#ifndef INDEX_HPP
#define INDEX_HPP

#include <vector>
#include <map>

#include "lexicon.hpp"
#include "posting.hpp"
#include "meta.hpp"
#include "extended_lexicon.hpp"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"
#include "static_index.hpp"

#define INDEXDIR "./disk_index/"//path to static index folders
#define PDIR "./disk_index/positional/"//path to static positional index
#define NPDIR "./disk_index/non_positional/"//path to static non-positional index
#define BLOCK 128

typedef std::map<unsigned int, std::vector<Posting>>::iterator P_ITE;
typedef std::map<unsigned int, std::vector<nPosting>>::iterator NP_ITE;
typedef std::vector<Posting>::iterator P_V;
typedef std::vector<nPosting>::iterator NP_V;

//This index does not use compression
class Index {
public:
    Index();
    void insert_document(std::string& url, std::string& newpage);

private:
    std::map<unsigned int, std::vector<Posting>> positional_index;
    std::map<unsigned int, std::vector<nPosting>> nonpositional_index;

    Structures::DocumentStore docstore;
    Structures::TranslationTable transtable;
    Lexicon lex;
    StaticIndex staticwriter;
};

#endif
