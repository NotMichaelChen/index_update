#include "extended_lexicon.hpp"

<<<<<<< HEAD
void ExtendedLexicon::addNonPositional( std::string term, mData entry ){
        exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( std::string term, mData entry ){
    exlexp[term].push_back(entry);
=======
#include <unordered_map>
#include <vector>

void ExtendedLexicon::addNonPositional( unsigned int term, mDatanp& entry ){
    exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( unsigned int term, mDatap& entry ){
    exlexp[term].push_back(entry);
}

mDatap ExtendedLexicon::getPositional(unsigned int term, int index) {
    return exlexp[term][index];
}

mDatanp ExtendedLexicon::getNonPositional(unsigned int term, int index) {
    return exlexnp[term][index];
}

std::vector<mDatap>::iterator ExtendedLexicon::getPositionalBegin(unsigned int term) {
    return exlexp[term].begin();
}

std::vector<mDatap>::iterator ExtendedLexicon::getPositionalEnd(unsigned int term) {
    return exlexp[term].end();
}

std::vector<mDatanp>::iterator ExtendedLexicon::getNonPositionalBegin(unsigned int term) {
    return exlexnp[term].begin();
}

std::vector<mDatanp>::iterator ExtendedLexicon::getNonPositionalEnd(unsigned int term) {
    return exlexnp[term].end();
>>>>>>> e1bf2a4af59bd032b60f645a07dabce190fba3d9
}
