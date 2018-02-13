#include "index.hpp"

#include <iomanip>
#include <sys/stat.h>

#include "doc_analyzer/analyzer.h"

#define POSTING_LIMIT 10000 //make sure doesn't exceed memory limit
#define BLOCKSIZE 128

Index::Index() : docstore(), transtable(), lex(), staticwriter("disk_index", BLOCKSIZE) {
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
}

void Index::display_non_positional(){
    std::cout << "***************** Displaying non positional **************** " << std::endl;
    for( std::map<unsigned int, std::vector<nPosting>>::iterator it = nonpositional_index.begin(); it != nonpositional_index.end(); it ++){
        std::cout << it->first << std::endl;
        for( std::vector<nPosting>::iterator vit = it->second.begin(); vit != it->second.end(); vit ++){
            std::cout << vit->docID << ' ' << vit->second << std::endl;
        }
    }
    std:: cout << "********************************" << std::endl;
}

void Index::display_positional(){
    std::cout << "***************** Displaying positional **************** " << std::endl;
    for( std::map<unsigned int, std::vector<Posting>>::iterator it = positional_index.begin(); it != positional_index.end(); it ++){
        std::cout << it->first << std::endl;
        for( std::vector<Posting>::iterator vit = it->second.begin(); vit != it->second.end(); vit ++){
            std::cout << vit->docID << ' ' << vit->second << ' '<< vit->third << std::endl;
        }
    }
    std:: cout << "********************************" << std::endl;
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
        insert_posting<nPosting>(nonpositional_index[entry.termid], posting);
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
        insert_posting<Posting>(positional_index[entry.termid], posting);
        ++positional_size;
        if(positional_size > POSTING_LIMIT) {
            //display_positional();
            staticwriter.write_p_disk(positional_index.begin(), positional_index.end());
            positional_index.clear();
            positional_size = 0;
        }
    }
}

template<typename T>
void Index::insert_posting(std::vector<T>& postinglist, T posting) {
    if(postinglist.empty()) {
        postinglist.push_back(posting);
        return;
    }

    int low = 0;
    int high = postinglist.size()-1;
    int mid = 0;

    while(true) {
        mid = low + (high - low) / 2;

        if(posting.docID < postinglist[mid].docID) {
            high = mid - 1;
            if(low > high) {
                break;
            }
        }
        else if(posting.docID > postinglist[mid].docID) {
            low = mid + 1;
            if(low > high) {
                mid += 1;
                break;
            }
        }
        else 
            break;
    }

    postinglist.insert(postinglist.begin() + mid, posting);
}