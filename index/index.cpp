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
#define BLOCKSIZE 128

Index::Index() : docstore(), transtable(), lex(), exlex(), staticwriter("disk_index", BLOCKSIZE) {
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
            staticwriter.write_np_disk(nonpositional_index.begin(), nonpositional_index.end());
            nonpositional_index.clear();

        }
    }

    //Insert P postings
    for(auto p_iter = results.Ppostings.begin(); p_iter != results.Ppostings.end(); p_iter++) {
        Lex_data entry = lex.getEntry(p_iter->term);

        Posting posting(entry.termid, p_iter->docID, p_iter->fragID, p_iter->pos);
        positional_index[entry.termid].push_back(posting);
        if(positional_index.size() > POSTING_LIMIT) {
            staticwriter.write_p_disk(positional_index.begin(), positional_index.end());
            positional_index.clear();
        }
    }
}
