/**
 * Lexicon is used to look up termID given a term. Thus, lexicon uses term as key, termID as value.
 * Dictionary in the indexer can also be called extended lexicon. It uses termID as key.
 */

#ifndef LEXICON_HPP
#define LEXICON_HPP

#include <string>
#include <fstream>

#include "libs/json.hpp"
#include "libs/sparsepp/spp.h"

struct Lex_data {
    Lex_data(unsigned int t, int f) : termid(t), f_t(f) {}
    unsigned int termid;
    int f_t; //How many documents the term appeared in
};

class Lexicon {
public:
    Lexicon();

    Lex_data& getEntry(std::string& term);

    void dump(std::ofstream& ofile);
    void restore(std::ifstream& ifile);

    void clear();
    size_t getSize();

private:
    spp::sparse_hash_map<std::string, Lex_data>::iterator initEntry(std::string& term);
    
    spp::sparse_hash_map<std::string, Lex_data> lex;
    unsigned int nextID;
};

#endif
