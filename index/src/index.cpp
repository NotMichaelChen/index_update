#include "index.hpp"

#include <sys/stat.h>
#include <fstream>

#include "util.hpp"
#include "query_processing/DAAT.hpp"
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

    size_t doccount = docstore.getDocumentCount();

    DAATStatData statistics = {doccount, &docscontaining, &doclength, avgdoclength};

    return DAAT(termIDs, nonpositional_index, *(staticwriter.getExLexPointer()), working_dir+GlobalConst::NonPosPath, statistics);
}

Index::Index(std::string directory) : docstore(), transtable(), lex(), staticwriter(directory) {
    working_dir = "./" + directory;

    //https://stackoverflow.com/a/4980833
    struct stat st;
    if(!(stat(working_dir.c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir(working_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!(stat((working_dir + GlobalConst::IndexPath).c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir((working_dir + GlobalConst::IndexPath).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!(stat((working_dir + GlobalConst::PosPath).c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir((working_dir + GlobalConst::PosPath).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!(stat((working_dir + GlobalConst::NonPosPath).c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir((working_dir + GlobalConst::NonPosPath).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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
        if(doclength.size() == 1)
            avgdoclength = 0;
        else
            avgdoclength = ((avgdoclength * doclength.size()) - newpage.length()) / (doclength.size() - 1);
    }
    //Add new value
    avgdoclength += (newpage.length() - avgdoclength) / (doclength.size()+1);
    doclength[results.docID] = newpage.length();

    std::cerr << "Got P:" << results.Ppostings.size() << " NP:" << results.NPpostings.size() << " Postings" << std::endl;

    insertNPPostings(results);
    insertPPostings(results);
}

void Index::insertNPPostings(MatcherInfo& results) {
    bool isFirstDoc = (results.se.getOldSize() == 0);
    //Insert NP postings
    for(auto np_iter = results.NPpostings.begin(); np_iter != results.NPpostings.end(); np_iter++) {
        Lex_data& entry = lex.getEntry(np_iter->second.term);

        //Update entry freq
        if(isFirstDoc)
            entry.f_t++;
        else
            //In old, not in new
            if(results.se.inOld(np_iter->second.term) && !results.se.inNew(np_iter->second.term))
                entry.f_t--;
            //In new, not in old
            else if(!results.se.inOld(np_iter->second.term) && results.se.inNew(np_iter->second.term))
                entry.f_t++;
            //Don't change in other cases

        GlobalType::NonPosIndex::iterator insertioniter;

        //Lookup where the posting list is in the index for the given termID
        auto iter_lookup = nonpositional_lookup.find(entry.termid);
        if(iter_lookup == nonpositional_lookup.end()) {
            //Construct posting list for the term since it doesn't exist
            auto results = nonpositional_index.emplace(std::make_pair(entry.termid, std::vector<nPosting>{}));
            nonpositional_lookup[entry.termid] = results.first;

            insertioniter = results.first;
        }
        else {
            insertioniter = iter_lookup->second;
        }

        insertioniter->second.emplace_back(entry.termid, np_iter->second.docID, np_iter->second.freq);
    }

    nonpositional_size += results.NPpostings.size();
    if(nonpositional_size > POSTING_LIMIT) {
        //when dynamic index cannot fit into memory, write to disk
        std::cerr << "Writing non-positional index" << std::endl;
        staticwriter.write_np_disk(nonpositional_index.begin(), nonpositional_index.end());
        nonpositional_lookup.clear();
        nonpositional_index.clear();
        nonpositional_size = 0;
    }
}

void Index::insertPPostings(MatcherInfo& results) {
    //Insert P postings
    for(auto p_iter = results.Ppostings.begin(); p_iter != results.Ppostings.end(); p_iter++) {
        Lex_data entry = lex.getEntry(p_iter->term);

        GlobalType::PosIndex::iterator insertioniter;

        //Lookup where the posting list is in the index for the given termID
        auto iter_lookup = positional_lookup.find(entry.termid);
        if(iter_lookup == positional_lookup.end()) {
            //Construct posting list for the term since it doesn't exist
            auto results = positional_index.emplace(std::make_pair(entry.termid, std::vector<Posting>{}));
            positional_lookup[entry.termid] = results.first;

            insertioniter = results.first;
        }
        else {
            insertioniter = iter_lookup->second;
        }

        insertioniter->second.emplace_back(entry.termid, p_iter->docID, p_iter->fragID, p_iter->pos);
    }

    positional_size += results.Ppostings.size();
    if(positional_size > POSTING_LIMIT) {
        std::cerr << "Writing positional index" << std::endl;
        staticwriter.write_p_disk(positional_index.begin(), positional_index.end());
        positional_lookup.clear();
        positional_index.clear();
        positional_size = 0;
    }
}

void Index::dump() {
    nlohmann::json jobject;

    //Write lexicons
    lex.dump(jobject);
    staticwriter.getExLexPointer()->dump(jobject);

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
    std::ofstream ofile(working_dir + "/indexdump", std::ios::out | std::ios::trunc);
    ofile.write(jstring.c_str(), jstring.size());

    redisDumpDatabase(working_dir + "/dump.rdb");
}

void Index::restore() {
    std::ifstream ifile(working_dir + "/indexdump");
    if(!ifile) {
        return;
    }

    nlohmann::json jobject;
    ifile >> jobject;

    //Read Lexicons
    lex.restore(jobject);
    staticwriter.getExLexPointer()->restore(jobject);

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

    redisRestoreDatabase(working_dir + "/dump.rdb");
}

void Index::clear() {
    positional_index.clear();
    nonpositional_index.clear();
    doclength.clear();
    avgdoclength = positional_size = nonpositional_size = 0;
    
    docstore.clear();
    transtable.clear();
    lex.clear();
    staticwriter.getExLexPointer()->clear();
}

void Index::printSize() {
    std::cerr << "positional: " << positional_size << std::endl;
    std::cerr << "non-positional: " << nonpositional_size << std::endl;
    
    std::cerr << "doclength: " << doclength.size() << std::endl;

    std::cerr << "lex: " << lex.getSize() << std::endl;

    std::cerr << "exlex:" << std::endl;
    staticwriter.getExLexPointer()->printSize();

    std::cerr << "inmemory avgdoclength: " << avgdoclength << std::endl;
    std::cerr << "redis avgdoclength: " << docstore.getAverageDocLength() << std::endl;
}