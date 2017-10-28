#ifndef INDEX_HPP
#define INDEX_HPP

#include <vector>
#include <map>

#include "lexicon.hpp"
#include "extended_lexicon.hpp"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

//This index does not use compression
class Index {
public:
    void Index();
    void insert_document(std::string& url, std::string& newpage);

    void write_np(int indexnum = 0, char prefix = 'a');
    void write_p(int indexnum, char prefix);
private:
    std::map<std::string, std::vector<Posting>> positional_index;
    std::map<std::string, std::vector<nPosting>> nonpositional_index;

	map<string, vector<f_meta>, strless> filemeta;//extended lexicon

    Lexicon lex;
    Structures::DocumentStore docstore;
    Structures::TranslationTable transtable;
};

#endif
