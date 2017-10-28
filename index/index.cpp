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
#define PDIR "./disk_index/positional/"//path to static positional index
#define NPDIR "./disk_index/non_positional/"//path to static non-positional index

Index::Index() {
    docstore = Structures::DocumentStore();
    transtable = Structures::TranslationTable();
    lex = Lexicon();
    exlex = ExtendedLexicon();
}

void Indexer::write_np(int indexnum, char prefix){
	/**
	 * Open a file to write to and store the metadata of a term.
	 */
    ofstream ofile;//non-positional inverted index
    string pdir(NPDIR);
    string namebase;
    string filename;
    if(prefix == 'a'){
        //TODO: default state, compressing from dynamic index, not from merging
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
                v_docID.push_back(it->docID);
    			v_freq.push_back(it->freq);
                v_sign.push_back(it->sign);
    			it ++;
            }

    		mmDatanp = compress_np(namebase, ofile, fm, v_docID, v_freq, v_sign);
            exlex.addNonPositional(currID, mmDatanp);
            currID = it->termID;

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
        if(np_iter->freq > 0 && !results.se.inOld(np_iter->term))
			lex.updateFreq(np_iter->term, 1);
		else if(np_iter->freq < 0 && !results.se.inNew(np_iter->term))
			lex.updateFreq(np_iter->term, -1);

        //Will always evaluate to either 0 or 1
        unsigned int sign = (np_iter->freq >= 0);
        unsigned int freq = std::abs(np_iter->freq);

        nPosting posting(entry.termid, np_iter->docID, freq, sign);
        nonpositional_index[np_iter->term].push_back(posting);
        if(nonpositional_index.size() > POSTING_LIMIT) {
            //when dynamic index cannot fit into memory, write to disk
            write_np();
            nonpositional_index.clear();
            merge_test();//TODO

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
