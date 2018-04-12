#include <iostream>
#include "extended_lexicon.hpp"

void ExtendedLexicon::addNonPositional( unsigned int term, mData& entry ){
    exlexnp[term].push_back(entry);
}

void ExtendedLexicon::addPositional( unsigned int term, mData& entry ){
    exlexp[term].push_back(entry);
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

void ExtendedLexicon::dump(nlohmann::json& jobject) {
    for(auto mapiter = exlexnp.begin(); mapiter != exlexnp.end(); mapiter++) {
        std::string key = std::to_string(mapiter->first);

        for(auto veciter = mapiter->second.begin(); veciter != mapiter->second.end(); veciter++) {
            jobject["npexlex"][key].push_back(nlohmann::json::object({
                {"filename", veciter->filename},
                {"start_pos", veciter->start_pos},
                {"last_docID", veciter->last_docID},
                {"blocksizes", veciter->blocksizes},
                {"postings_blocks", veciter->postings_blocks},
                {"end_offset", veciter->end_offset}
            }));
        }
    }

    for(auto mapiter = exlexp.begin(); mapiter != exlexp.end(); mapiter++) {
        std::string key = std::to_string(mapiter->first);

        for(auto veciter = mapiter->second.begin(); veciter != mapiter->second.end(); veciter++) {
            jobject["pexlex"][key].push_back(nlohmann::json::object({
                {"filename", veciter->filename},
                {"start_pos", veciter->start_pos},
                {"last_docID", veciter->last_docID},
                {"blocksizes", veciter->blocksizes},
                {"postings_blocks", veciter->postings_blocks},
                {"end_offset", veciter->end_offset}
            }));
        }
    }
}

void ExtendedLexicon::restore(nlohmann::json& jobject) {
    auto jiter = jobject.find("npexlex");
    if(jiter != jobject.end()) {
        for(auto lexiter = jiter->begin(); lexiter != jiter->end(); lexiter++) {
            unsigned int key = std::stoul(lexiter.key());
            std::vector<mData> data;

            for(auto metaiter = lexiter->begin(); metaiter != lexiter->end(); metaiter++) {
                mData tempdata;
                tempdata.filename = metaiter->at("filename");
                tempdata.start_pos = metaiter->at("start_pos");
                tempdata.last_docID = metaiter->at("last_docID");
                tempdata.blocksizes = metaiter->at("blocksizes");
                tempdata.postings_blocks = metaiter->at("postings_blocks");
                tempdata.end_offset = metaiter->at("end_offset");
                data.push_back(tempdata);
            }

            exlexnp.insert(std::make_pair(key, data));
        }
    }

    jiter = jobject.find("pexlex");
    if(jiter != jobject.end()) {
        for(auto lexiter = jiter->begin(); lexiter != jiter->end(); lexiter++) {
            unsigned int key = std::stoul(lexiter.key());
            std::vector<mData> data;

            for(auto metaiter = lexiter->begin(); metaiter != lexiter->end(); metaiter++) {
                mData tempdata;
                tempdata.filename = metaiter->at("filename");
                tempdata.start_pos = metaiter->at("start_pos");
                tempdata.last_docID = metaiter->at("last_docID");
                tempdata.blocksizes = metaiter->at("blocksizes");
                tempdata.postings_blocks = metaiter->at("postings_blocks");
                tempdata.end_offset = metaiter->at("end_offset");
                data.push_back(tempdata);
            }

            exlexp.insert(std::make_pair(key, data));
        }
    }
}