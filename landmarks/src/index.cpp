#include "index.hpp"

#include "doc_parsing/docparser.hpp"
#include "utility/util.hpp"

void Index::insert_document(std::string& url, std::string& newpage) {
    std::string timestamp = Utility::getTimestamp();
    DocumentParser parser(url, newpage, timestamp, docstore);

    insertNPPostings(parser);
    insertPPostings(parser);
    auto landmarks = parser.getLandmarks();
    landdir.insertLandmarkArray(parser.getDocID(), landmarks);
}

void Index::insertNPPostings(DocumentParser& results) {
    auto NPpostings = results.getNPPostings();

    //Insert NP postings
    for(auto np_iter = NPpostings.begin(); np_iter != NPpostings.end(); np_iter++) {
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

        nonpositional_index[entry.termid].emplace_back(entry.termid, np_iter->docID, np_iter->freq);
        ++nonpositional_size;
    }

    // if(nonpositional_size > POSTING_LIMIT) {
    //     //when dynamic index cannot fit into memory, write to disk
    //     std::cerr << "Writing non-positional index" << std::endl;
    //     staticwriter.write_np_disk(nonpositional_index.begin(), nonpositional_index.end());
    //     nonpositional_index.clear();
    //     nonpositional_size = 0;
    // }
}

void Index::insertPPostings(DocumentParser& results) {
    auto Ppostings = results.getPPostings();

    //Insert P postings
    for(auto p_iter = Ppostings.begin(); p_iter != Ppostings.end(); p_iter++) {
        Lex_data entry = lex.getEntry(p_iter->term);

        positional_index[entry.termid].emplace_back(entry.termid, p_iter->docID, p_iter->landID, p_iter->offset);
        ++positional_size;
    }

    // if(positional_size > POSTING_LIMIT) {
    //     std::cerr << "Writing positional index" << std::endl;
    //     staticwriter.write_p_disk(positional_index.begin(), positional_index.end());
    //     positional_index.clear();
    //     positional_size = 0;
    // }
}