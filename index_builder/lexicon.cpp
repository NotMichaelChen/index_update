#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include "lexicon.hpp"

using namespace std;

Lexicon::Lexicon() : nextID(0) {}

Lex_data Lexicon::getEntry(string& term) {
    if(lex.find(term) == lex.end()) {
        initEntry(term);
    }
    return lex[term];
}

void Lexicon::updateFreq(string& term, int delta) {
    if(lex.find(term) == lex.end()) {
        initEntry(term);
    }
    lex[term].f_t += delta;
}

void Lexicon::updatePositional(string& term, int delta) {
    if(lex.find(term) == lex.end()) {
        initEntry(term);
    }
    lex[term].p_ptr += delta;
}

void Lexicon::updateNonPositional(string& term, int delta) {
    if(lex.find(term) == lex.end()) {
        initEntry(term);
    }
    lex[term].np_ptr += delta;
}

void Lexicon::initEntry(string& term) {
    auto iter = lex.find(term);
    //Only init the entry if it does not exist
    if(iter == lex.end()) {
        Lex_data entry;
        entry.termid = nextID;
        nextID++;
        lex[term] = entry;
    }
}
