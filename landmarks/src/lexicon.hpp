#pragma once

#include <string>

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

private:
    spp::sparse_hash_map<std::string, Lex_data>::iterator initEntry(std::string& term);
    
    spp::sparse_hash_map<std::string, Lex_data> lex;
    unsigned int nextID;
};