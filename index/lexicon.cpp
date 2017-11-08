#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include "lexicon.hpp"

using namespace std;

Lexicon::Lexicon() : nextID(0) {}

Lex_data Lexicon::getEntry(string& term) {
    if(lex.find(term) == lex.end()) {
        initEntry(term);
    }
    return lex[term];
}

void Lexicon::updateFreq(string& term, int new_freq) {
    if(lex.find(term) == lex.end()) {
        initEntry(term);
    }
    lex[term].f_t = new_freq;
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

void Lexicon::dump() {
    ofstream dumpfile;
    dumpfile.open("lexdump");

    for(auto iter = lex.begin(); iter != lex.end(); ++iter) {
        dumpfile << iter->first << " ";
        dumpfile << iter->second.termid << " ";
        dumpfile << iter->second.f_t << " ";
    }

    dumpfile.close();
}

bool Lexicon::restore() {
    ifstream dumpfile;
    dumpfile.open("lexdump");

    if(!dumpfile)
        return false;

    string line;
    size_t convertlen;

    stringstream linebuf;

    while(getline(dumpfile, line)) {
        linebuf = stringstream(line);
        string key;
        Lex_data entry;
        linebuf >> key >> entry.termid >> entry.f_t;
        lex[key] = entry;
    }

    dumpfile.close();

    return true;
}
