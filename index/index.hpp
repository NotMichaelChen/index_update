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
    template<typename T>
    void insert_posting(std::vector<T>& postinglist, T posting);

    std::map<unsigned int, std::vector<Posting>> positional_index;
    std::map<unsigned int, std::vector<nPosting>> nonpositional_index;
    //TODO: Is it possible to not use separate variables for these?
    unsigned long positional_size;
    unsigned long nonpositional_size;

    Structures::DocumentStore docstore;
    Structures::TranslationTable transtable;
    Lexicon lex;
    StaticIndex staticwriter;
};

#endif
