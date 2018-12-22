#include "index_builder.hpp"
#include "doc_partitioner/partitioner.hpp"
#include "utility/string_util.hpp"
#include "utility/md5.h"
#include "global_constants.hpp"
#include "global_constants.hpp"

#include <string>
#include <vector>
#include <utility>
#include <sys/stat.h>

IndexBuilder::IndexBuilder(const std::string& directory, const unsigned int b, const unsigned int w) : winnower(b, w), currdid(0),
                                                                        lex(), dynamicindex(), staticwriter(directory) {
    working_dir = "./" + directory;

    //https://stackoverflow.com/a/4980833
    struct stat st;
    if(!(stat(working_dir.c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir(working_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!(stat((working_dir + IndexPath).c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir((working_dir + IndexPath).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!(stat((working_dir + PosPath).c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir((working_dir + PosPath).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!(stat((working_dir + NonPosPath).c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir((working_dir + NonPosPath).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
}

void IndexBuilder::insertDocument(const std::string& pageurl, const std::string& pagecontent) {
    std::vector<Fragment> fragments = winnower.partitionPage(currdid, pagecontent);
    std::cout << "Partition finished: " << fragments.size() << " fragments\n";

    if (this->pagetable.find(pageurl) != this->pagetable.end()) {
        size_t newver = this->dvtable.updateDocVersion(this->pagetable.at(pageurl), fragments);
        std::cout << "New version: "<< newver << '\n';
    } else {
        size_t newdid = this->dvtable.addDocVersion(fragments);
        this->pagetable.insert(std::make_pair(pageurl, newdid));
        currdid++;
    }
    std::cout << dvtable.size() << ' ' << pagetable.size() << '\n';

    for (const Fragment& currfrag : fragments) {
        std::string md5frag = md5(currfrag.fragcontent);
        FragID currfragid = std::make_pair(currfrag.docid, currfrag.startpos);
        if (fragmenthashtable.find(md5frag) != fragmenthashtable.end()) {
            std::cout << "Duplicate frag found! (" << currfrag.docid << ' ' << currfrag.startpos << ")\n";
            continue;
        }
        fragmenthashtable.insert(std::make_pair(md5frag, currfragid));
        this->insertFragment(currfrag);
    }
}

void IndexBuilder::insertFragment(const Fragment& currfrag) {
    std::cout << "Inserting fragment (" << currfrag.docid << ", " << currfrag.startpos << ")\n";

    std::vector<std::string> fragwords = StringUtil::splitString(currfrag.fragcontent, ' ');

    std::unordered_map<std::string, size_t> freqcount;
    for (const std::string& s : fragwords) {
        if (freqcount.find(s) == freqcount.end()) { freqcount[s] = 0; }
        freqcount[s] += 1;
    }

    for (size_t i = 0; i < fragwords.size(); ++i) {
        Lex_data entry = lex.getEntry(fragwords[i]);
        this->dynamicindex.insertPostingNP(entry.termid, currfrag.docid, freqcount[fragwords[i]]);
        this->dynamicindex.insertPostingP(entry.termid, currfrag.docid, currfrag.startpos, i);
    }

    if(dynamicindex.getPPostingCount() > POSTING_LIMIT) {
        std::cerr << "Writing positional index" << '\n';
        auto positers = dynamicindex.getPosIter();
        staticwriter.write_p_disk(positers.first, positers.second);
        dynamicindex.clearPos();
    }
    if(dynamicindex.getNPPostingCount() > POSTING_LIMIT) {
        std::cerr << "Writing non-positional index" << std::endl;
        auto nonpositers = dynamicindex.getNonPosIter();
        staticwriter.write_np_disk(nonpositers.first, nonpositers.second);
        dynamicindex.clearNonPos();
    }
}