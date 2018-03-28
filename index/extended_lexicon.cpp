#include <iostream>
#include <fstream>

#include <sstream>

#include "extended_lexicon.hpp"

void ExtendedLexicon::addNonPositional( unsigned int term, mData& entry ){
    exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( unsigned int term, mData& entry ){
    exlexp[term].push_back(entry);
}

void ExtendedLexicon::dump() {
    std::ofstream dumpfile;

    dumpfile.open("pexlexdump", std::ios::out | std::ios::trunc);
    for(auto iter = exlexp.begin(); iter != exlexp.end(); ++iter) {
        dumpfile << iter->first << std::endl;
        for(auto metaiter = iter->second.begin(); metaiter != iter->second.end(); ++metaiter) {
            dumpfile << "\t";
            dumpfile << metaiter->filename << " ";
            dumpfile << metaiter->start_pos << " ";
            dumpfile << metaiter->last_docID << " ";
            dumpfile << metaiter->blocksizes << " ";
            dumpfile << metaiter->postings_blocks << " ";
            dumpfile << metaiter->end_offset << std::endl;
        }
    }
    dumpfile.close();

    dumpfile.open("npexlexdump", std::ios::out | std::ios::trunc);
    for(auto iter = exlexnp.begin(); iter != exlexnp.end(); ++iter) {
        dumpfile << iter->first << std::endl;
        for(auto metaiter = iter->second.begin(); metaiter != iter->second.end(); ++metaiter) {
            dumpfile << "\t";
            dumpfile << metaiter->filename << " ";
            dumpfile << metaiter->start_pos << " ";
            dumpfile << metaiter->last_docID << " ";
            dumpfile << metaiter->blocksizes << " ";
            dumpfile << metaiter->postings_blocks << " ";
            dumpfile << metaiter->end_offset << std::endl;
        }
    }
    dumpfile.close();
}

bool ExtendedLexicon::restore() {
    std::ifstream dumpfile;
    dumpfile.open("plexdump");

    if(!dumpfile)
        return false;

    std::string line;
    std::stringstream linebuf;
    while(std::getline(dumpfile, line)) {
        unsigned int term = stoul(line);

        std::vector<mData> metavec;
        while(dumpfile.peek() == '\t') {
            mData metadata;
            std::getline(dumpfile, line);
            linebuf = std::stringstream(line);

            dumpfile >> metadata.filename;
            dumpfile >> metadata.start_pos;
            dumpfile >> metadata.last_docID;
            dumpfile >> metadata.blocksizes;
            dumpfile >> metadata.postings_blocks;
            dumpfile >> metadata.end_offset;

            metavec.push_back(metadata);
        }

        exlexp[term] = metavec;
    }

    dumpfile.close();
    dumpfile.clear();

    dumpfile.open("nplexdump");

    if(!dumpfile)
        return false;

    while(std::getline(dumpfile, line)) {
        unsigned int term = stoul(line);

        std::vector<mData> metavec;
        while(dumpfile.peek() == '\t') {
            mData metadata;
            std::getline(dumpfile, line);
            linebuf = std::stringstream(line);

            dumpfile >> metadata.filename;
            dumpfile >> metadata.start_pos;
            dumpfile >> metadata.last_docID;
            dumpfile >> metadata.blocksizes;
            dumpfile >> metadata.postings_blocks;
            dumpfile >> metadata.end_offset;

            metavec.push_back(metadata);
        }

        exlexnp[term] = metavec;
    }

    dumpfile.close();

    return true;
}

std::vector<mData>::iterator ExtendedLexicon::deleteNonPositional(unsigned int term, std::vector<mData>::iterator entry) {
    return exlexnp[term].erase(entry);
}

std::vector<mData>::iterator ExtendedLexicon::deletePositional(unsigned int term, std::vector<mData>::iterator entry) {
    return exlexp[term].erase(entry);
}

std::vector<mData>::iterator ExtendedLexicon::getPositional(unsigned int term, std::string filename) {
    for( std::vector<mData>::iterator it = exlexp[term].begin(); it != exlexp[term].end(); it ++){
        if( it->filename == filename ) return it;
    }
    throw std::invalid_argument("(P) Error, extended lexicon does not have term " + std::to_string(term) + " in file " + filename);
}

std::vector<mData>::iterator ExtendedLexicon::getNonPositional(unsigned int term, std::string filename) {
    for( std::vector<mData>::iterator it = exlexnp[term].begin(); it != exlexnp[term].end(); it ++){
        if( it->filename == filename ) return it;
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
