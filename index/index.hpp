#ifndef INDEX_HPP
#define INDEX_HPP

#include <vector>
#include <map>
#include <unordered_map>

#include "lexicon.hpp"
#include "posting.hpp"
#include "meta.hpp"
#include "extended_lexicon.hpp"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"
#include "static_index.hpp"
#include "global_parameters.hpp"

//This index does not use compression
class Index {
public:
    Index();
    void insert_document(std::string& url, std::string& newpage);
    //Temporary return type: returns docIDs for now
    std::vector<unsigned int> query(std::vector<std::string> words);

private:
    template<typename T>
    void insert_posting(std::vector<T>& postinglist, T posting);

    //Data structures
    GlobalType::PosIndex positional_index;
    GlobalType::NonPosIndex nonpositional_index;
    //TODO: Can this be obtained from docstore?
    std::unordered_map<uint64_t, uint32_t> doclength;
    double avgdoclength;

    //TODO: Is it possible to not use separate variables for these?
    unsigned long positional_size;
    unsigned long nonpositional_size;

    Structures::DocumentStore docstore;
    Structures::TranslationTable transtable;
    Lexicon lex;
    StaticIndex staticwriter;
};

#endif
