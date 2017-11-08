#include "extended_lexicon.hpp"

void ExtendedLexicon::addNonPositional( std::string term, mData entry ){
        exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( std::string term, mData entry ){
    exlexp[term].push_back(entry);
}
