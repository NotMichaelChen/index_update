#include "lexicon.hpp"

using namespace std;

Lexicon::Lexicon() : nextID(0) {}

Lex_data& Lexicon::getEntry(string& term) {
    auto iter = lex.find(term);
    if(iter == lex.end()) {
        iter = initEntry(term);
    }
    return iter->second;
}

//term must *NOT* exist inside of the lexicon already
spp::sparse_hash_map<std::string, Lex_data>::iterator Lexicon::initEntry(string& term) {
    auto results = lex.emplace(term, Lex_data{nextID, 0});
    nextID++;
    return results.first;
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

void Lexicon::clear() {
    lex.clear();
    nextID = 0;
}

size_t Lexicon::getSize() {
    return lex.size();
}