#include "lexicon.hpp"
#include <iostream>
#include <fstream>

using namespace std;

Lexicon::Lexicon() : nextID(0) {}

Lex_data Lexicon::getEntry(string& term) {
    if(lex.find(term) == lex.end()) {
        initEntry(term);
    }
    return lex[term];
}

void Lexicon::display(){
    for(std::map<std::string, Lex_data>::iterator it = lex.begin(); it != lex.end(); it ++){
        std::cout << it->first << ' '<< it->second.termid << ' ' << it->second.f_t << std::endl;
    }
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
        entry.f_t = 0;
        nextID++;
        lex[term] = entry;
    }
}

void Lexicon::dump() {
    ofstream dumpfile;
    dumpfile.open("lexdump", ios::out | ios::trunc);

    dumpfile << nextID << endl;

    for(auto iter = lex.begin(); iter != lex.end(); ++iter) {
        dumpfile << iter->first << " ";
        dumpfile << iter->second.termid << " ";
        dumpfile << iter->second.f_t << " ";
        dumpfile << endl;
    }

    dumpfile.close();
}

bool Lexicon::restore() {
    ifstream dumpfile;
    dumpfile.open("lexdump");

    if(!dumpfile)
        return false;

    string line;

    getline(dumpfile, line);
    nextID = stoul(line);

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
