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

void Lexicon::dump() {
    ofstream dumpfile;
    dumpfile.open("lexdump");

    dumpfile << lex.size() << "\n";
    for(auto iter = lex.begin(); iter != lex.end(); ++iter) {
        dumpfile << iter->first << " ";
        dumpfile << iter->second.termid << " ";
        dumpfile << iter->second.f_t << " ";
        dumpfile << iter->second.p_ptr << " ";
        dumpfile << iter->second.np_ptr << "\n";
    }

    dumpfile.close();
}

bool Lexicon::restore() {
    ifstream dumpfile;
    dumpfile.open("lexdump")

    if(!dumpfile)
        return false;

    string line;
    size_t convertlen;

    getline(dumpfile, line);
    lex.size = stoi(line, &convertlen);
    if(convertlen == 0)
        return false;

    stringstream linebuf;

    while(getline(dumpfile, line)) {
        linebuf = stringstream(line);
        string key;
        Lex_data entry;
        linebuf >> key >> entry.termid >> entry.f_t >> entry.p_ptr >> entry.np_ptr;
        lex[key] = entry;
    }

    dumpfile.close();

    return true;
}
