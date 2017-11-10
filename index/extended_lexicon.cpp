#include "extended_lexicon.hpp"
#include <unordered_map>
#include <vector>

void ExtendedLexicon::addNonPositional( unsigned int term, mData& entry ){
    exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( unsigned int term, mData& entry ){
    exlexp[term].push_back(entry);
}

mData ExtendedLexicon::getPositional(unsigned int term, std::string filename) {
    for( std::vector<mData>::iterator it = exlexp[term].begin(); it != exlexp[term].end(); it ++){
        if( it->filename == filename ) return *it;
    }
    return mData{""};
}

mData ExtendedLexicon::getNonPositional(unsigned int term, std::string filename) {
    for( std::vector<mData>::iterator it = exlexnp[term].begin(); it != exlexnp[term].end(); it ++){
        if( it->filename == filename ) return *it;
    }
    return mData{""};
}

std::vector<mData>::iterator ExtendedLexicon::getPositionalBegin(unsigned int term) {
    return exlexp[term].begin();
}

std::vector<mData>::iterator ExtendedLexicon::getPositionalEnd(unsigned int term) {
    return exlexp[term].end();
}

std::vector<mData>::iterator ExtendedLexicon::getNonPositionalBegin(unsigned int term) {
    return exlexnp[term].begin();
}

std::vector<mData>::iterator ExtendedLexicon::getNonPositionalEnd(unsigned int term) {
    return exlexnp[term].end();
}
