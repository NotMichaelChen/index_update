#include "index.hpp"

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>

#include "doc_analyzer/analyzer.h"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

#define POSTING_LIMIT 500 //make sure doesn't exceed memory limit

Index::Index() {
    docstore = Structures::DocumentStore();
    transtable = Structures::TranslationTable();
    lex = Lexicon();
}

void Index::insert_document(std::string& url, std::string& newpage) {
    //Get timestamp, https://stackoverflow.com/a/16358111
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto timestamp = oss.str();
    
    MatcherInfo results = indexUpdate(url, newpage, timestamp, docstore, transtable);

    //Insert NP postings
    for(auto np_iter = results.NPpostings.begin(); np_iter != results.NPpostings.end(); np_iter++) {
        Lex_data entry = lex.getEntry(np_iter->term);

        //Update entry freq
        lex.updateFreq(np_iter->term, np_iter->freq);

        //Will always evaluate to either 0 or 1
        unsigned int sign = (np_iter->freq >= 0);
        unsigned int freq = std::abs(np_iter->freq);
        
        nPosting posting(entry.termid, np_iter->docID, freq, sign);
        nonpositional_index[np_iter->term].push_back(posting);
        if(nonpositional_index.size() > POSTING_LIMIT) {
            //TODO: do something here
        }
    }

    //Insert P postings
    for(auto p_iter = results.Ppostings.begin(); p_iter != results.Ppostings.end(); p_iter++) {
        Lex_data entry = lex.getEntry(p_iter->term);

        Posting posting(entry.termid, p_iter->docID, p_iter->fragID, p_iter->pos);
        positional_index[p_iter->term].push_back(posting);
        if(positional_index.size() > POSTING_LIMIT) {
            //TODO: do something here
        }
    }
}

