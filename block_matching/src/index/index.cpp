#include "index.hpp"

#include <sys/stat.h>
#include <fstream>

#include "utility/util.hpp"
#include "query_processing/DAAT.hpp"

std::vector<unsigned int> Index::query(std::vector<std::string> words) {
    std::vector<unsigned int> termIDs;
    std::vector<unsigned int> docscontaining;
    for(size_t i = 0; i < words.size(); ++i) {
        std::transform(words[i].begin(), words[i].end(), words[i].begin(), ::tolower);
        Lex_data entry = lex.getEntry(words[i]);
        termIDs.push_back(entry.termid);
        docscontaining.push_back(entry.f_t);
    }

    throw std::runtime_error("Not Implemented");
    //TODO: fix QP to use iterators instead of the whole index
    // return DAAT(termIDs, docscontaining, nonpositional_index, *(staticwriter.getExLexPointer()),
    //     working_dir+GlobalConst::NonPosPath, docstore);
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
}

void Index::insert_document(std::string& url, std::string& newpage) {
    std::string timestamp = Utility::getTimestamp();

    //Perform document analysis
    MatcherInfo results = indexUpdate(url, newpage, timestamp, docstore, transtable);

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
        
        dynamicindex.insertPostingNP(entry.termid, np_iter->second.docID, np_iter->second.freq);
    }

    if(dynamicindex.getNPPostingCount() > POSTING_LIMIT) {
        //when dynamic index cannot fit into memory, write to disk
        std::cerr << "Writing non-positional index" << std::endl;
        auto nonpositers = dynamicindex.getNonPosIter();
        staticwriter.write_np_disk(nonpositers.first, nonpositers.second);
        dynamicindex.clearNonPos();
    }
}

void Index::insertPPostings(MatcherInfo& results) {
    //Insert P postings
    for(auto p_iter = results.Ppostings.begin(); p_iter != results.Ppostings.end(); p_iter++) {
        Lex_data entry = lex.getEntry(p_iter->term);

        this->dynamicindex.insertPostingP(entry.termid, p_iter->docID, p_iter->fragID, p_iter->pos);
    }

    if(dynamicindex.getPPostingCount() > POSTING_LIMIT) {
        std::cerr << "Writing positional index" << std::endl;
        auto positers = dynamicindex.getPosIter();
        staticwriter.write_p_disk(positers.first, positers.second);
        dynamicindex.clearPos();
    }
}

void Index::dump() {
    //TODO: reimplement
    dynamicindex.dump(staticwriter);

    std::ofstream ofile(working_dir + "/lexicon", std::ios::binary);
    lex.dump(ofile);
    ofile.close();
    ofile.clear();

    ofile.open(working_dir + "/extendedlexicon", std::ios::binary);
    staticwriter.getExLexPointer()->dump(ofile);
    ofile.close();
}

void Index::restore() {
    //TODO: reimplement

    std::ifstream ifile(working_dir + "/lexicon", std::ios::binary);
    lex.restore(ifile);
    ifile.close();
    ifile.clear();

    ifile.open(working_dir + "/extendedlexicon", std::ios::binary);
    staticwriter.getExLexPointer()->restore(ifile);
    ifile.close();
}

void Index::clear() {
    dynamicindex.clear();
    docstore.clear();
    transtable.clear();
    lex.clear();
    staticwriter.getExLexPointer()->clear();
}

void Index::printSize() {
    std::cerr << "positional: " << dynamicindex.getPPostingCount() << std::endl;
    std::cerr << "non-positional: " << dynamicindex.getNPPostingCount() << std::endl;

    std::cerr << "lex: " << lex.getSize() << std::endl;

    std::cerr << "exlex:" << std::endl;
    staticwriter.getExLexPointer()->printSize();

    std::cerr << "redis avgdoclength: " << docstore.getAverageDocLength() << std::endl;
}