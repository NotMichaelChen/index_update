#include "index.hpp"

#include <sys/stat.h>
#include <fstream>

#include "utility/util.hpp"
#include "utility/redis.hpp"

Index::Index(std::string directory) : docstore(), landdir(), lex(), staticwriter(directory) {
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
    DocumentParser parser(url, newpage, timestamp, docstore, landdir);

    insertNPPostings(parser);
    insertPPostings(parser);
}

void Index::insertNPPostings(DocumentParser& results) {
    //Insert NP postings
    std::vector<ExternNPposting> nppostings = results.getNPPostings();
    for(auto np_iter = nppostings.begin(); np_iter != nppostings.end(); np_iter++) {
        Lex_data& entry = lex.getEntry(np_iter->term);

        //Update entry freq
        if(results.isFirstDoc())
            entry.f_t++;
        else
            //In old, not in new
            if(results.termInOld(np_iter->term) && !results.termInNew(np_iter->term))
                entry.f_t--;
            //In new, not in old
            else if(!results.termInOld(np_iter->term) && results.termInNew(np_iter->term))
                entry.f_t++;
            //Don't change in other cases

        dynamicindex.insertPostingNP(entry.termid, np_iter->docID, np_iter->freq);
    }

    if(dynamicindex.getNPPostingCount() > POSTING_LIMIT) {
        //when dynamic index cannot fit into memory, write to disk
        std::cerr << "Writing non-positional index" << std::endl;
        auto nonpositers = dynamicindex.getNonPosIter();
        staticwriter.write_np_disk(nonpositers.first, nonpositers.second);
        dynamicindex.clearNonPos();
    }
}

void Index::insertPPostings(DocumentParser& results) {
    //Insert P postings
    std::vector<ExternPposting> ppostings = results.getPPostings();
    for(auto p_iter = ppostings.begin(); p_iter != ppostings.end(); p_iter++) {
        Lex_data entry = lex.getEntry(p_iter->term);

        this->dynamicindex.insertPostingP(entry.termid, p_iter->docID, p_iter->landID, p_iter->offset);
    }

    if(dynamicindex.getPPostingCount() > POSTING_LIMIT) {
        std::cerr << "Writing positional index" << std::endl;
        auto positers = dynamicindex.getPosIter();
        staticwriter.write_p_disk(positers.first, positers.second);
        dynamicindex.clearPos();
    }
}

void Index::dump() {
    dynamicindex.dump(staticwriter);

    std::ofstream ofile(working_dir + "/lexicon", std::ios::binary);
    lex.dump(ofile);
    ofile.close();
    ofile.clear();

    ofile.open(working_dir + "/extendedlexicon", std::ios::binary);
    staticwriter.getExLexPointer()->dump(ofile);
    ofile.close();

    std::string redisname = working_dir + "/redisdump_docstore";
    //TODO: don't use hardcoded numbers
    redis_dump(redisname, 0);
    redisname = working_dir + "/redisdump_docstoremeta";
    redis_dump(redisname, 2);
}

void Index::restore() {
    std::ifstream ifile(working_dir + "/lexicon", std::ios::binary);
    lex.restore(ifile);
    ifile.close();
    ifile.clear();

    ifile.open(working_dir + "/extendedlexicon", std::ios::binary);
    staticwriter.getExLexPointer()->restore(ifile);
    ifile.close();

    std::string redisname = working_dir + "/redisdump_docstore";
    //TODO: don't use hardcoded numbers
    redis_restore(redisname, 0);
    redisname = working_dir + "/redisdump_docstoremeta";
    redis_restore(redisname, 2);
}

void Index::clear() {
    dynamicindex.clear();
    docstore.clear();
    lex.clear();
    staticwriter.getExLexPointer()->clear();
    redis_flushDB();
}

void Index::printSize() {
    std::cerr << "positional: " << dynamicindex.getPPostingCount() << std::endl;
    std::cerr << "non-positional: " << dynamicindex.getNPPostingCount() << std::endl;

    std::cerr << "lex: " << lex.getSize() << std::endl;

    std::cerr << "exlex:" << std::endl;
    staticwriter.getExLexPointer()->printSize();

    std::cerr << "redis avgdoclength: " << docstore.getAverageDocLength() << std::endl;

    std::cerr << "landmark directory: " << landdir.totalSize() << std::endl;
}