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
        std::vector<unsigned int> v_docID;
    	std::vector<unsigned int> v_fragID;
    	std::vector<unsigned int> v_pos;
    	mData mmData;
    	unsigned int num_of_p = 0;//number of posting of a certain term

        auto it = positional_index.begin();
        string currterm = it->first;
    	while( it != positional_index.end() ){
    		while(it->first == currterm && it != positional_index.end()){
    			v_docID.push_back(it->second->docID);
    			v_fragID.push_back(it->second->fragID);
    			v_pos.push_back(it->second->pos);
    			it ++;
    			num_of_p ++;
    		}

    		mmData = compress_p(namebase, ofile, fm, v_docID, v_fragID, v_pos);
    		mmData.num_posting = num_of_p;

            exlex.addPositional(currterm, mmData);
            currterm = it->first;

    		num_of_p = 0;
    		v_docID.clear();
    		v_fragID.clear();
    		v_pos.clear();
    	}
    	ofile.close();
    }else{
        cerr << "File cannot be opened." << endl;
    }
}

void Index::write_np(int indexnum, char prefix){
	/**
	 * Open a file to write to and store the metadata of a term.
	 */
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
        //cout << "Compressing and writing to " << namebase << endl;

        std::vector<unsigned int> v_docID;
    	std::vector<unsigned int> v_freq;
        std::vector<unsigned int> v_sign;
    	mDatanp mmDatanp;

        unsigned int currID = 0;

        auto it = nonpositional_index.begin();
        while( it != nonpositional_index.end() ){
            currID = it->first;
            while( it->first == currID ){
                v_docID.push_back(it->second->docID);
    			v_freq.push_back(it->second->freq);
                v_sign.push_back(it->second->sign);
    			it ++;
            }

    		mmDatanp = compress_np(namebase, ofile, fm, v_docID, v_freq, v_sign);
            exlex.addNonPositional(currID, mmDatanp);
            currID = it->first;

    		v_docID.clear();
    		v_freq.clear();
    		v_sign.clear();
    	}

    	ofile.close();
    }
    else
        cerr << "File cannot be opened." << endl;
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
            //TODO: do something here
            write_p();
            positional_index.clear();
            merge_test();
        }
    }
}
