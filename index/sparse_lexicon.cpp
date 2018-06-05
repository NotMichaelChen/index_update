#include "sparse_lexicon.hpp"

void SparseExtendedLexicon::insertPosEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset, unsigned int length) {
    std::vector<std::unordered_map<unsigned int, ExtendedLexiconEntry>>& lex = zposlex;

    if(!isZindex)
        lex = iposlex;

    if(indexnum >= lex.size())
        lex.resize(indexnum);
    
    lex[indexnum].emplace(termID, offset, length);
}

void SparseExtendedLexicon::insertNonPosEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset, unsigned int length) {
    std::vector<std::unordered_map<unsigned int, ExtendedLexiconEntry>>& lex = znonposlex;

    if(!isZindex)
        lex = inonposlex;

    if(indexnum >= lex.size())
        lex.resize(indexnum);
    
    lex[indexnum].emplace(termID, offset, length);
}

unsigned long SparseExtendedLexicon::getPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex) {
    std::vector<std::unordered_map<unsigned int, ExtendedLexiconEntry>>& lex = zposlex;

    if(!isZindex)
        lex = iposlex;

    if(indexnum >= lex.size() || lex[indexnum].find(termID) == lex[indexnum].end())
        throw std::invalid_argument("Error, no entry exists for termID " + std::to_string(termID) + " in pos index " + std::to_string(indexnum));
    
    return lex[indexnum][termID].offset;
}

unsigned long SparseExtendedLexicon::getNonPosOffset(unsigned int termID, unsigned int indexnum, bool isZindex) {
    std::vector<std::unordered_map<unsigned int, ExtendedLexiconEntry>>& lex = znonposlex;

    if(!isZindex)
        lex = inonposlex;

    if(indexnum >= lex.size() || lex[indexnum].find(termID) == lex[indexnum].end())
        throw std::invalid_argument("Error, no entry exists for termID " + std::to_string(termID) + " in nonpos index " + std::to_string(indexnum));
    
    return lex[indexnum][termID].offset;
}