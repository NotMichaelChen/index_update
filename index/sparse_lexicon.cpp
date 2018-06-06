#include "sparse_lexicon.hpp"

void SparseExtendedLexicon::insertEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset,
    bool positional)
{
    std::vector<std::unordered_map<unsigned int, unsigned long>>& lex = zposlex;

    if(positional) {
        //Don't need to check for positive case since default is zposlex
        if(!isZindex)
            lex = iposlex;
    }
    else {
        if(isZindex)
            lex = znonposlex;
        else
            lex = inonposlex;
    }

    if(indexnum >= lex.size())
        lex.resize(indexnum+1);
    lex[indexnum].emplace(std::make_pair(termID, offset));
}

void SparseExtendedLexicon::clearIndex(unsigned int indexnum, bool positional) {
    if(positional) {
        if(indexnum < zposlex.size())
            zposlex[indexnum].clear();

        if(indexnum < iposlex.size())
            iposlex[indexnum].clear();
    }
    else {
        if(indexnum < znonposlex.size())
            znonposlex[indexnum].clear();
            
        if(indexnum < inonposlex.size())
            inonposlex[indexnum].clear();
    }
}

unsigned long SparseExtendedLexicon::getPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex) {
    std::vector<std::unordered_map<unsigned int, unsigned long>>& lex = zposlex;

    if(!isZindex)
        lex = iposlex;

    if(indexnum >= lex.size() || lex[indexnum].find(termID) == lex[indexnum].end())
        throw std::invalid_argument("Error, no entry exists for termID " + std::to_string(termID) + " in pos index " + std::to_string(indexnum));
    
    return lex[indexnum][termID];
}

unsigned long SparseExtendedLexicon::getNonPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex) {
    std::vector<std::unordered_map<unsigned int, unsigned long>>& lex = znonposlex;

    if(!isZindex)
        lex = inonposlex;

    if(indexnum >= lex.size() || lex[indexnum].find(termID) == lex[indexnum].end())
        throw std::invalid_argument("Error, no entry exists for termID " + std::to_string(termID) + " in nonpos index " + std::to_string(indexnum));
    
    return lex[indexnum][termID];
}