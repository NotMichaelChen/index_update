#include "extended_lexicon.hpp"

void ExtendedLexicon::addNonPositional( std::string term, mDatanp& entry ){
    exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( std::string term, mDatap& entry ){
    exlexp[term].push_back(entry);
}

mDatap ExtendedLexicon::getPositional(std::string term, int index) {
    return exlexp[term][index];
}

mDatanp ExtendedLexicon::getNonPositional(std::string term, int index) {
    return exlexnp[term][index];
}
