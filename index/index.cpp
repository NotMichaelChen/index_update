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

//TODO: why commented out
Index::Index() {
    docstore = Structures::DocumentStore();
    transtable = Structures::TranslationTable();
    lex = Lexicon();
    exlex = ExtendedLexicon();
}

void Index::write_p(int indexnum, char prefix){
    std::ofstream ofile;//positional inverted index
    std::string pdir(PDIR);
    std::string namebase;
    std::string filename;
    if(prefix == 'a'){
        filename = pdir + "Z" + std::to_string(indexnum);
        ofile.open(filename, std::ofstream::app | std::ofstream::binary);

        if(ofile.tellp() != 0){
            std::cout << filename << " already exists." << std::endl;
            ofile.close();
            filename = pdir + "I" + std::to_string(indexnum);
            ofile.open(filename, std::ios::ate | std::ios::binary);
            namebase = std::string("I") + std::to_string(indexnum);
        }else{
            namebase = std::string("Z") + std::to_string(indexnum);
        }
    }else{
        filename = pdir + prefix + std::to_string(indexnum);
        ofile.open(filename, std::ofstream::app | std::ofstream::binary);
        namebase = prefix+ std::to_string(indexnum);
    }

    if (ofile.is_open()){
        P_ITE ite = positional_index.begin();
        P_ITE end = positional_index.end();
        P_V vit = ite->second.begin();
        P_V vend = ite->second.end();
        compress_posting<P_ITE, P_V>(namebase, ofile, ite, end, vit, vend, 1);

    	ofile.close();
    }else{
        std::cerr << "File cannot be opened." << std::endl;
    }
}

void Index::write_np(int indexnum, char prefix){
    /**
     * Open a file to write to and store the metadata of a term.
     */
    std::ofstream ofile;//non-positional inverted index
    std::string pdir(NPDIR);
    std::string namebase;
    std::string filename;
    if(prefix == 'a'){
        //default state, compressing from dynamic index, not from merging
        filename = pdir + "Z" + std::to_string(indexnum);
        ofile.open(filename, std::ofstream::app | std::ofstream::binary);

        if(ofile.tellp() != 0){
            std::cout << filename << " already exists." << std::endl;
            ofile.close();
            filename = pdir + "I" + std::to_string(indexnum);
            ofile.open(filename, std::ios::ate | std::ios::binary);
            namebase = std::string("I") + std::to_string(indexnum);
        }else{
            namebase = std::string("Z") + std::to_string(indexnum);
        }
    }else{
        filename = pdir + prefix + std::to_string(indexnum);
        ofile.open(filename, std::ofstream::app | std::ofstream::binary);
        namebase = prefix+ std::to_string(indexnum);
    }

    if (ofile.is_open()){
        NP_ITE ite = nonpositional_index.begin();
        NP_ITE end = nonpositional_index.end();
        NP_V vit = ite->second.begin();
        NP_V vend = ite->second.end();
        compress_posting<NP_ITE, NP_V>(namebase, ofile, ite, end, vit, vend, 0);

    	ofile.close();
    }
    else std::cerr << "File cannot be opened." << std::endl;
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

        nPosting posting(entry.termid, np_iter->docID, np_iter->freq);
        nonpositional_index[entry.termid].push_back(posting);
        if(nonpositional_index.size() > POSTING_LIMIT) {
            //when dynamic index cannot fit into memory, write to disk
            write_np();
            nonpositional_index.clear();
            merge_test();

        }
    }

    //Insert P postings
    for(auto p_iter = results.Ppostings.begin(); p_iter != results.Ppostings.end(); p_iter++) {
        Lex_data entry = lex.getEntry(p_iter->term);

        Posting posting(entry.termid, p_iter->docID, p_iter->fragID, p_iter->pos);
        positional_index[entry.termid].push_back(posting);
        if(positional_index.size() > POSTING_LIMIT) {
            write_p();
            positional_index.clear();
            merge_test();
        }
    }
}
