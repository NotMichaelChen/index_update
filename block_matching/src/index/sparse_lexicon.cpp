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

void SparseExtendedLexicon::dump(nlohmann::json& jobject) {
    for(size_t i = 0; i < zposlex.size(); i++) {
        for(auto mapiter = zposlex[i].begin(); mapiter != zposlex[i].end(); mapiter++) {
            jobject["zposlex"][i][std::to_string(mapiter->first)] = mapiter->second;
        }
    }

    for(size_t i = 0; i < iposlex.size(); i++) {
        for(auto mapiter = iposlex[i].begin(); mapiter != iposlex[i].end(); mapiter++) {
            jobject["iposlex"][i][std::to_string(mapiter->first)] = mapiter->second;
        }
    }

    for(size_t i = 0; i < znonposlex.size(); i++) {
        for(auto mapiter = znonposlex[i].begin(); mapiter != znonposlex[i].end(); mapiter++) {
            jobject["znonposlex"][i][std::to_string(mapiter->first)] = mapiter->second;
        }
    }

    for(size_t i = 0; i < inonposlex.size(); i++) {
        for(auto mapiter = inonposlex[i].begin(); mapiter != inonposlex[i].end(); mapiter++) {
            jobject["inonposlex"][i][std::to_string(mapiter->first)] = mapiter->second;
        }
    }
}

void SparseExtendedLexicon::restore(nlohmann::json& jobject) {
    auto jiter = jobject.find("zposlex");
    if(jobject.find("zposlex") != jobject.end()) {
        zposlex.resize(jiter->size());
        for(size_t i = 0; i < jiter->size(); i++) {
            for(auto mapiter = jiter->at(i).begin(); mapiter != jiter->at(i).end(); mapiter++) {
                unsigned int key = std::stoul(mapiter.key());
                zposlex[i].insert(std::make_pair(key, mapiter.value()));
            }
        }
    }

    jiter = jobject.find("iposlex");
    if(jobject.find("iposlex") != jobject.end()) {
        iposlex.resize(jiter->size());
        for(size_t i = 0; i < jiter->size(); i++) {
            for(auto mapiter = jiter->at(i).begin(); mapiter != jiter->at(i).end(); mapiter++) {
                unsigned int key = std::stoul(mapiter.key());
                iposlex[i].insert(std::make_pair(key, mapiter.value()));
            }
        }
    }

    jiter = jobject.find("znonposlex");
    if(jobject.find("znonposlex") != jobject.end()) {
        znonposlex.resize(jiter->size());
        for(size_t i = 0; i < jiter->size(); i++) {
            for(auto mapiter = jiter->at(i).begin(); mapiter != jiter->at(i).end(); mapiter++) {
                unsigned int key = std::stoul(mapiter.key());
                znonposlex[i].insert(std::make_pair(key, mapiter.value()));
            }
        }
    }

    jiter = jobject.find("inonposlex");
    if(jobject.find("inonposlex") != jobject.end()) {
        inonposlex.resize(jiter->size());
        for(size_t i = 0; i < jiter->size(); i++) {
            for(auto mapiter = jiter->at(i).begin(); mapiter != jiter->at(i).end(); mapiter++) {
                unsigned int key = std::stoul(mapiter.key());
                inonposlex[i].insert(std::make_pair(key, mapiter.value()));
            }
        }
    }
}

void SparseExtendedLexicon::clear() {
    zposlex.clear();
    znonposlex.clear();
    iposlex.clear();
    inonposlex.clear();
}