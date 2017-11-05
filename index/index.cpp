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
#include "compressor.hpp"

#define POSTING_LIMIT 500 //make sure doesn't exceed memory limit
#define PDIR "./disk_index/positional/"//path to static positional index
#define NPDIR "./disk_index/non_positional/"//path to static non-positional index

typedef std::map<std::string, std::vector<Posting>>::iterator P_ITE;
typedef std::map<std::string, std::vector<nPosting>>::iterator NP_ITE;

Index::Index() {
    docstore = Structures::DocumentStore();
    transtable = Structures::TranslationTable();
    lex = Lexicon();
    exlex = ExtendedLexicon();
}

void Index::write_p(int indexnum, char prefix){
	ofstream ofile;//positional inverted index
    string pdir(PDIR);
    string namebase;
    string filename;
    if(prefix == 'a'){
        filename = pdir + "Z" + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);

    	if(ofile.tellp() != 0){
            cout << filename << " already exists." << endl;
    		ofile.close();
    		filename = pdir + "I" + to_string(indexnum);
            ofile.open(filename, ios::ate | ios::binary);
            namebase = string("I") + to_string(indexnum);
    	}else{
            namebase = string("Z") + to_string(indexnum);
        }
    }else{
        filename = pdir + prefix + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);
        namebase = prefix+ to_string(indexnum);
    }

    if (ofile.is_open()){
        P_ITE ite = positional_index.begin();
        compress_field<P_ITE>(namebase, ofile, ite, 1);

    	ofile.close();
    }else{
        cerr << "File cannot be opened." << endl;
    }
}

void Index::write_np(int indexnum, char prefix){
    //TODO: unify prefix to Z and I
    ofstream ofile;//non-positional inverted index
    string pdir(NPDIR);
    string namebase;
    string filename;
    if(prefix == 'a'){
        //default state, compressing from dynamic index, not from merging
        filename = pdir + "X" + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);

    	if(ofile.tellp() != 0){
            cout << filename << " already exists." << endl;
    		ofile.close();
    		filename = pdir + "L" + to_string(indexnum);
            ofile.open(filename, ios::ate | ios::binary);
            namebase = string("L") + to_string(indexnum);
    	}else{
            namebase = string("X") + to_string(indexnum);
        }
    }else{
        filename = pdir + prefix + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);
        namebase = prefix+ to_string(indexnum);
    }

    if (ofile.is_open()){
        NP_ITE ite = nonpositional_index.begin();
        compress_field<NP_ITE>(namebase, ofile, ite, 0);

    	ofile.close();
    }
    else cerr << "File cannot be opened." << endl;
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
        nonpositional_index[np_iter->term].push_back(posting);
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
        positional_index[p_iter->term].push_back(posting);
        if(positional_index.size() > POSTING_LIMIT) {
            //TODO: do something here
            write_p();
            positional_index.clear();
            merge_test();
        }
    }
}
