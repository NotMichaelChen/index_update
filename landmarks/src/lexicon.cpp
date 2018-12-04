#include "lexicon.hpp"

Lexicon::Lexicon() : nextID(0) {}

Lex_data& Lexicon::getEntry(std::string& term) {
    auto iter = lex.find(term);
    if(iter == lex.end()) {
        iter = initEntry(term);
    }
    return iter->second;
}

//term must *NOT* exist inside of the lexicon already
spp::sparse_hash_map<std::string, Lex_data>::iterator Lexicon::initEntry(std::string& term) {
    auto results = lex.emplace(term, Lex_data{nextID, 0});
    nextID++;
    return results.first;
}