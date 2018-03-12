#include "DAAT.hpp"

std::priority_queue<double, std::vector<double>, std::greater<double>> DAAT(std::vector<unsigned int> docIDs, 
    std::map<unsigned int, std::vector<nPosting>>& index, ExtendedLexicon& exlex)
{
    //TODO: Catch empty docID case
    std::vector<query_primitive> listpointers;

    for(int i : docIDs) {
        listpointers.emplace_back(i, index, exlex);
    }

    unsigned int did = 0;

    //TODO: Replace condition here with something reasonable
    while(true) {
        did = listpointers[0].nextGEQ(did);

        unsigned int d = 0;
        for(int i = 1; (d = listpointers[i].nextGEQ(did)) == did; i++)
            ;

        if(d > did)
            did = d;
        else {
            std::vector<unsigned int> freqs;
            /* docID is in intersection; now get all frequencies */
            for(size_t i = 0; i < listpointers.size(); i++)
                freqs.push_back(listpointers[i].getFreq());
            /* compute BM25 score from frequencies and other data */
            //<details omitted>
            did++; /* and increase did to search for next post */
        }
    }
}