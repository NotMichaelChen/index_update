#include "index.hpp"

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>

#include "doc_analyzer/analyzer.h"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

#define POSTING_LIMIT 500 //make sure doesn't exceed memory limit

Index::Index() : docstore(), transtable(), lex(), exlex() {
    //https://stackoverflow.com/a/4980833
    struct stat st;
    if(!(stat(INDEXDIR,&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir(INDEXDIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!(stat(PDIR,&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir(PDIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!(stat(NPDIR,&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir(NPDIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
}

//Writes the positional index to disk, which means it is saved either in file Z0 or I0.
void Index::write_p() {
    std::string filename = PDIR;
    //Z0 exists
    if(std::ifstream(filename + "Z0"))
        filename += "I0";
    else
        filename += "Z0";

    std::ofstream ofile(filename);

    if (ofile.is_open()){
        P_ITE ite = positional_index.begin();
        P_ITE end = positional_index.end();
        P_V vit = ite->second.begin();
        P_V vend = ite->second.end();
        compress_posting<P_ITE, P_V>(filename, ofile, ite, end, vit, vend, 1);

        ofile.close();
    }else{
        std::cerr << "File cannot be opened." << std::endl;
    }
}

void Index::write_np() {
    std::string filename = NPDIR;
    //Z0 exists
    if(std::ifstream(filename + "Z0"))
        filename += "I0";
    else
        filename += "Z0";

    std::ofstream ofile(filename);

    if (ofile.is_open()){
        NP_ITE ite = nonpositional_index.begin();
        NP_ITE end = nonpositional_index.end();
        NP_V vit = ite->second.begin();
        NP_V vend = ite->second.end();
        compress_posting<NP_ITE, NP_V>(filename, ofile, ite, end, vit, vend, 0);

        ofile.close();
    }else{
        std::cerr << "File cannot be opened." << std::endl;
    }
}

void Index::insert_document(std::string& url, std::string& newpage) {
    //Get timestamp, https://stackoverflow.com/a/16358111
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto timestamp = oss.str();

    MatcherInfo results = indexUpdate(url, newpage, timestamp, docstore, transtable);

    std::cerr << "Got P:" << results.Ppostings.size() << " NP:" << results.NPpostings.size() << " Postings" << std::endl;

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
            merge_test(false);

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
            merge_test(true);
        }
    }
}
