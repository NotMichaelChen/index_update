#include "sparse_lexicon.hpp"

#include <iostream>

void SparseExtendedLexicon::insertEntry(unsigned int termID, unsigned int indexnum, bool isZindex, unsigned long offset,
    bool positional)
{
    std::vector<std::map<unsigned int, unsigned long>>* lex = &zposlex;

    if(positional) {
        //Don't need to check for positive case since default is zposlex
        if(!isZindex)
            lex = &iposlex;
    }
    else {
        if(isZindex)
            lex = &znonposlex;
        else
            lex = &inonposlex;
    }

    if(indexnum >= lex->size())
        lex->resize(indexnum+1);
    (*lex)[indexnum].emplace(std::make_pair(termID, offset));
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

//Get the offset of the nearest termID less than or equal to the given termID
unsigned long SparseExtendedLexicon::getPosLEQOffset(unsigned int termID, unsigned int indexnum, bool isZindex) {
    std::vector<std::map<unsigned int, unsigned long>>* lex = &zposlex;

    if(!isZindex)
        lex = &iposlex;

    if(indexnum >= lex->size())
        throw std::invalid_argument("Error, invalid pos index number: " + std::to_string(indexnum));
    if((*lex)[indexnum].empty())
        throw std::invalid_argument("Error, trying to query empty index: " + std::to_string(indexnum));
    
    auto iter = (*lex)[indexnum].upper_bound(termID);
    
    //Subtract to get the actual closest LEQ entry
    if(iter != (*lex)[indexnum].begin())
        iter--;
    return iter->second;
}

//Get the offset of the nearest termID less than or equal to the given termID
unsigned long SparseExtendedLexicon::getNonPosLEQOffset(unsigned int termID, unsigned int indexnum, bool isZindex) {
    std::vector<std::map<unsigned int, unsigned long>>* lex = &znonposlex;

    if(!isZindex)
        lex = &inonposlex;

    if(indexnum >= lex->size())
        throw std::invalid_argument("Error, invalid pos index number: " + std::to_string(indexnum));
    if((*lex)[indexnum].empty())
        throw std::invalid_argument("Error, trying to query empty index: " + std::to_string(indexnum));
    
    auto iter = (*lex)[indexnum].upper_bound(termID);
    
    //Subtract to get the actual closest LEQ entry
    if(iter != (*lex)[indexnum].begin())
        iter--;
    return iter->second;
}

void SparseExtendedLexicon::printSize() {
    unsigned long counter = 0;
    for(auto& entry : zposlex) {
        counter += entry.size();
    }
    std::cerr << "zposlex: " << counter << std::endl;
    counter = 0;
    for(auto& entry : iposlex) {
        counter += entry.size();
    }
    std::cerr << "iposlex: " << counter << std::endl;
    counter = 0;
    for(auto& entry : znonposlex) {
        counter += entry.size();
    }
    std::cerr << "znonposlex: " << counter << std::endl;
    counter = 0;
    for(auto& entry : inonposlex) {
        counter += entry.size();
    }
    std::cerr << "inonposlex: " << counter << std::endl;
}