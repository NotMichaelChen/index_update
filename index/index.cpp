#include "index.hpp"

#include <iomanip>
#include <sys/stat.h>
#include <algorithm>

#include "doc_analyzer/analyzer.h"
#include "global_parameters.hpp"
#include "util.hpp"
#include "query_processing/DAAT.hpp"
#include "json.hpp"
#include "redis.hpp"

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
    //TODO: make disk_index folder public
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
    std::string timestamp = Utility::getTimestamp();

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
        nonpositional_index[entry.termid].push_back(posting);
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
        positional_index[entry.termid].push_back(posting);
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

    //Write lexicons
    lex.dump(jobject);
    staticwriter.getExlexPointer()->dump(jobject);

    //Write individual variables
    jobject["positional_size"] = positional_size;
    jobject["nonpositional_size"] = nonpositional_size;
    jobject["avgdoclength"] = avgdoclength;
    for(auto iter = doclength.begin(); iter != doclength.end(); iter++) {
        std::string key = std::to_string(iter->first);
        jobject["doclength"][key] = iter->second;
    }

    //Write in-memory indexes
    for(auto inditer = nonpositional_index.begin(); inditer != nonpositional_index.end(); inditer++) {
        std::string key = std::to_string(inditer->first);
        for(auto postiter = inditer->second.begin(); postiter != inditer->second.end(); postiter++) {
            jobject["nonposindex"][key].push_back(nlohmann::json::object({
                {"termID", postiter->termID},
                {"docID", postiter->docID},
                {"frequency", postiter->second},
            }));
        }
    }

    for(auto inditer = positional_index.begin(); inditer != positional_index.end(); inditer++) {
        std::string key = std::to_string(inditer->first);
        for(auto postiter = inditer->second.begin(); postiter != inditer->second.end(); postiter++) {
            jobject["posindex"][key].push_back(nlohmann::json::object({
                {"termID", postiter->termID},
                {"docID", postiter->docID},
                {"fragmentID", postiter->second},
                {"position", postiter->third}
            }));
        }
    }

    std::string jstring = jobject.dump();
    std::ofstream ofile("indexdump", std::ios::out | std::ios::trunc);
    ofile.write(jstring.c_str(), jstring.size());

    redisDumpDatabase("dump.rdb");
}

void Index::restore() {
    std::ifstream ifile("indexdump");
    if(!ifile) {
        return;
    }

    nlohmann::json jobject;
    ifile >> jobject;

    //Read Lexicons
    lex.restore(jobject);
    staticwriter.getExlexPointer()->restore(jobject);

    //Read individual variables
    positional_size = jobject["positional_size"];
    nonpositional_size = jobject["nonpositional_size"];
    avgdoclength = jobject["avgdoclength"];
    for(auto dociter = jobject["doclength"].begin(); dociter != jobject["doclength"].end(); dociter++) {
        unsigned int key = std::stoul(dociter.key());
        doclength[key] = dociter.value();
    }

    //Read in-memory indexes
    auto jiter = jobject.find("nonposindex");
    if(jiter != jobject.end()) {
        for(auto inditer = jiter->begin(); inditer != jiter->end(); inditer++) {
            unsigned int key = std::stoul(inditer.key());
            std::vector<nPosting> data;

            for(auto dataiter = inditer->begin(); dataiter != inditer->end(); dataiter++) {
                data.emplace_back(dataiter->at("termID"), dataiter->at("docID"), dataiter->at("frequency"));
            }

            nonpositional_index[key] = data;
        }
    }

    jiter = jobject.find("posindex");
    if(jiter != jobject.end()) {
        for(auto inditer = jiter->begin(); inditer != jiter->end(); inditer++) {
            unsigned int key = std::stoul(inditer.key());
            std::vector<Posting> data;

            for(auto dataiter = inditer->begin(); dataiter != inditer->end(); dataiter++) {
                data.emplace_back(
                    dataiter->at("termID"),
                    dataiter->at("docID"),
                    dataiter->at("fragmentID"),
                    dataiter->at("position"));
            }

            positional_index[key] = data;
        }
    }

    redisRestoreDatabase("dump.rdb");
}