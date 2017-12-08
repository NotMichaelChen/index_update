#include <iostream>
#include "extended_lexicon.hpp"

void ExtendedLexicon::addNonPositional( unsigned int term, mData& entry ){
    exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( unsigned int term, mData& entry ){
    exlexp[term].push_back(entry);
}

void ExtendedLexicon::displayNonPositional(){
    for( std::unordered_map<unsigned int, std::vector<mData>>::iterator it = exlexnp.begin(); it != exlexnp.end(); it ++){
        std::cout << it->first << std::endl;
        for( std::vector<mData>::iterator vit = it->second.begin(); vit != it->second.end(); vit ++){
            std::cout << vit->filename << " ";
        }
        std::cout << std::endl;
    }
}

mData ExtendedLexicon::getPositional(unsigned int term, std::string filename) {
    for( std::vector<mData>::iterator it = exlexp[term].begin(); it != exlexp[term].end(); it ++){
        if( it->filename == filename ) return *it;
    }
    throw std::invalid_argument("(P) Error, extended lexicon does not have term " + std::to_string(term) + " in file " + filename);
}

mData ExtendedLexicon::getNonPositional(unsigned int term, std::string filename) {
    for( std::vector<mData>::iterator it = exlexnp[term].begin(); it != exlexnp[term].end(); it ++){
        if( it->filename == filename ) return *it;
    }
    throw std::invalid_argument("(NP) Error, extended lexicon does not have term " + std::to_string(term) + " in file " + filename);
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
