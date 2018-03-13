#include "DAAT.hpp"

GlobalType::DoubleMinHeap DAAT(std::vector<unsigned int> docIDs, GlobalType::NonPosIndex& index, ExtendedLexicon& exlex)
{
    if(docIDs.empty()) {
        return GlobalType::DoubleMinHeap();
    }

    std::vector<query_primitive> listpointers;

    //Construct listpointers for each docID
    for(unsigned int i : docIDs) {
        listpointers.emplace_back(i, index, exlex);
    }

    unsigned int did = 0;

    //TODO: Replace condition here with something reasonable
    while(did < GlobalConst::UIntMax) {
        did = listpointers[0].nextGEQ(did);

        unsigned int d = 0;
        for(int i = 1; i < listpointers.size() && (d = listpointers[i].nextGEQ(did)) == did; i++)
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