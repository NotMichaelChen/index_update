#include "index.hpp"

#include <iomanip>
#include <sys/stat.h>
#include <algorithm>

#include "doc_analyzer/analyzer.h"
#include "global_parameters.hpp"
#include "util.hpp"
#include "query_processing/DAAT.hpp"
#include "json.hpp"

//Get timestamp, https://stackoverflow.com/a/16358111
std::string getTimestamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    return oss.str();
}

std::vector<unsigned int> Index::query(std::vector<std::string> words) {
    std::vector<unsigned int> termIDs;
    std::vector<unsigned int> docscontaining;
    for(size_t i = 0; i < words.size(); ++i) {
        std::transform(words[i].begin(), words[i].end(), words[i].begin(), ::tolower);
        Lex_data entry = lex.getEntry(words[i]);
        termIDs.push_back(entry.termid);
        docscontaining.push_back(entry.f_t);
    }

    int doccount = docstore.getDocumentCount();

    DAATStatData statistics = {doccount, &docscontaining, &doclength, avgdoclength};

    return DAAT(termIDs, nonpositional_index, *(staticwriter.getExlexPointer()), statistics);
}

Index::Index() : docstore(), transtable(), lex(), staticwriter() {
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

    positional_size = 0;
    nonpositional_size = 0;
    avgdoclength = 0;
}

void Index::insert_document(std::string& url, std::string& newpage) {
    std::string timestamp = getTimestamp();

    //Perform document analysis
    MatcherInfo results = indexUpdate(url, newpage, timestamp, docstore, transtable);

    //Update document length info + average document length
    //https://math.stackexchange.com/a/1567342
    if(doclength.find(results.docID) != doclength.end()) {
        //Remove old value
        avgdoclength = ((avgdoclength * doclength.size()) - newpage.length()) / (doclength.size() - 1);
    }
    //Add new value
    avgdoclength += (newpage.length() - avgdoclength) / (doclength.size()+1);
    doclength[results.docID] = newpage.length();

    std::cerr << "Got P:" << results.Ppostings.size() << " NP:" << results.NPpostings.size() << " Postings" << std::endl;

    bool isFirstDoc = (results.se.getOldSize() == 0);
    //Insert NP postings
    for(auto np_iter = results.NPpostings.begin(); np_iter != results.NPpostings.end(); np_iter++) {
        Lex_data entry = lex.getEntry(np_iter->term);

        //Update entry freq
        //TODO: Refactor updatefreq method to be more convenient to use
        if(isFirstDoc) {
            lex.updateFreq(np_iter->term, entry.f_t+1);
        }
        else {
            //In old, not in new
            if(results.se.inOld(np_iter->term) && !results.se.inNew(np_iter->term)) {
                lex.updateFreq(np_iter->term, entry.f_t-1);
            }
            //In new, not in old
            else if(!results.se.inOld(np_iter->term) && results.se.inNew(np_iter->term)) {
                lex.updateFreq(np_iter->term, entry.f_t+1);
            }
            //Don't change in other cases
        }

        nPosting posting(entry.termid, np_iter->docID, np_iter->freq);
        Utility::binaryInsert<nPosting>(nonpositional_index[entry.termid], posting);
        ++nonpositional_size;
        if(nonpositional_size > POSTING_LIMIT) {
            //when dynamic index cannot fit into memory, write to disk
            //display_non_positional();
            staticwriter.write_np_disk(nonpositional_index.begin(), nonpositional_index.end());
            nonpositional_index.clear();
            nonpositional_size = 0;
        }
    }

    //Insert P postings
    for(auto p_iter = results.Ppostings.begin(); p_iter != results.Ppostings.end(); p_iter++) {
        Lex_data entry = lex.getEntry(p_iter->term);

        Posting posting(entry.termid, p_iter->docID, p_iter->fragID, p_iter->pos);
        Utility::binaryInsert<Posting>(positional_index[entry.termid], posting);
        ++positional_size;
        if(positional_size > POSTING_LIMIT) {
            //display_positional();
            staticwriter.write_p_disk(positional_index.begin(), positional_index.end());
            positional_index.clear();
            positional_size = 0;
        }
    }
}

void Index::dump() {
    nlohmann::json jobject;
    lex.dump(jobject);

    std::string jstring = jobject.dump();
    std::ofstream ofile("indexdump", std::ios::out | std::ios::trunc);
    ofile.write(jstring.c_str(), jstring.size());

}

void Index::restore() {
    std::ifstream ifile("indexdump");
    if(!ifile) {
        return;
    }

    nlohmann::json jobject;
    ifile >> jobject;

    lex.restore(jobject);
}