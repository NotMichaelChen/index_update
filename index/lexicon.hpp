/**
 * Lexicon is used to look up termID given a term. Thus, lexicon uses term as key, termID as value.
 * Dictionary in the indexer can also be called extended lexicon. It uses termID as key.
 */

#ifndef LEXICON_HPP
#define LEXICON_HPP

#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>

#include "json.hpp"
#include "sparsepp/spp.h"

struct Lex_data {
    Lex_data(unsigned int t, int f) : termid(t), f_t(f) {}
    unsigned int termid;
    int f_t; //How many documents the term appeared in
};

class Lexicon {
public:
    Lexicon();

    Lex_data getEntry(std::string& term);
    void updateFreq(std::string& term, int new_freq);

    //Dumps contents of lexicon into given json, under the object "lexicon"
    void dump(nlohmann::json& jobject);
    //Restores the lexicon from a given json object
    void restore(nlohmann::json& jobject);

    void clear();
    size_t getSize();

private:
    void initEntry(std::string& term);
    spp::sparse_hash_map<std::string, Lex_data> lex;
    //std::unordered_map<std::string, Lex_data> lex;
    unsigned int nextID;
};

#endif
