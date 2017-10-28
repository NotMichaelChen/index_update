#include "extended_lexicon.hpp"

void ExtendedLexicon::addNonPositional( unsigned int termID, mDatanp& entry ){
        exlexnp[termID].push_back(entry);
}
