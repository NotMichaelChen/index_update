#include "lexicon.hpp"
#include <iostream>
#include <fstream>

using namespace std;

Lexicon::Lexicon() : nextID(0) {}

Lex_data Lexicon::getEntry(string& term) {
    auto iter = lex.find(term);
    if(iter == lex.end()) {
        initEntry(term);
        //Re-get iterator
        iter = lex.find(term);
    }
    return iter->second;
}

void Lexicon::display(){
    for(std::map<std::string, Lex_data>::iterator it = lex.begin(); it != lex.end(); it ++){
        std::cout << it->first << ' '<< it->second.termid << ' ' << it->second.f_t << std::endl;
    }
}

void Lexicon::updateFreq(string& term, int new_freq) {
    auto iter = lex.find(term);
    if(iter == lex.end()) {
        initEntry(term);
        //Re-get iterator
        iter = lex.find(term);
    }
    iter->second.f_t = new_freq;
}

void Lexicon::initEntry(string& term) {
    auto iter = lex.find(term);
    //Only init the entry if it does not exist
    if(iter == lex.end()) {
        lex.emplace(term, Lex_data{nextID, 0});
        nextID++;
    }
}

//Adds lexicon data to the json object for later dumping to disk
void Lexicon::dump(nlohmann::json& jobject) {
    for(auto iter = lex.begin(); iter != lex.end(); ++iter) {
        jobject["lexicon"][iter->first]["termid"] = iter->second.termid;
        jobject["lexicon"][iter->first]["f_t"] = iter->second.f_t;
    }
}

void Lexicon::restore(nlohmann::json& jobject) {
    auto jiter = jobject.find("lexicon");
    if(jiter != jobject.end()) {
        for(auto lexiter = jiter->begin(); lexiter != jiter->end(); lexiter++) {
            lex.emplace(lexiter.key(), Lex_data{lexiter.value()["termid"], lexiter.value()["f_t"]});
        }
    }
}
