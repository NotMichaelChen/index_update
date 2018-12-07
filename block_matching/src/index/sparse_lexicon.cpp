#include "sparse_lexicon.hpp"

#include <iostream>
#include "static_functions/bytesIO.hpp"

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

void SparseExtendedLexicon::dump(std::ofstream& ofile) {

    //Helper function that writes a single sparse lexicon to disk
    auto writeLex = [&ofile](std::vector<std::map<unsigned int, unsigned long>>& lex) {
        // Write out number of entries in exlex
        writeAsBytes(lex.size(), ofile);

        for(const auto& lexicon : lex) {
            // Write out size of dictionary
            writeAsBytes(lexicon.size(), ofile);
            for(auto iter = lexicon.begin(); iter != lexicon.end(); iter++) {
                writeAsBytes(iter->first, ofile);
                writeAsBytes(iter->second, ofile);
            }
        }
    };

    writeLex(zposlex);
    writeLex(znonposlex);
    writeLex(iposlex);
    writeLex(inonposlex);
}

void SparseExtendedLexicon::restore(std::ifstream& ifile) {
    if(!ifile)
        return;

    auto readLex = [&ifile](std::vector<std::map<unsigned int, unsigned long>>& lex) {
        //Read in lexicon length
        size_t lexlen;
        readFromBytes(lexlen, ifile);
        for(size_t lexindex = 0; lexindex < lexlen; lexindex++) {
            size_t dictlen;
            readFromBytes(dictlen, ifile);
            for(size_t j = 0; j < dictlen; j++) {
                unsigned int key;
                unsigned long val;
                readFromBytes(key, ifile);
                readFromBytes(val, ifile);
                lex[lexindex][key] = val;
            }
        }
    };

    readLex(zposlex);
    readLex(znonposlex);
    readLex(iposlex);
    readLex(inonposlex);

}

void SparseExtendedLexicon::clear() {
    zposlex.clear();
    znonposlex.clear();
    iposlex.clear();
    inonposlex.clear();
}