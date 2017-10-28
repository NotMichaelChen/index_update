#include "extended_lexicon.hpp"

void ExtendedLexicon::addNonPositional( std::string term, mDatanp& entry ){
        exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( std::string term, mDatap& entry ){
    exlenxp[term].push_back(etnry);
}
