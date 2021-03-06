#include "DAAT.hpp"

#include "query_primitive.hpp"
#include "ranking_functions/BM25.hpp"

struct ScorePair {
    ScorePair() {}
    ScorePair(unsigned int d, double s) : docID(d), score(s) {}
    
    unsigned int docID;
    double score;
};

class greater_ScorePair {
public:
    bool operator()(ScorePair lhs, ScorePair rhs) {
        return lhs.score > rhs.score;
    }
};

std::vector<unsigned int> DAAT(std::vector<unsigned int>& termIDs, std::vector<unsigned int>& docscontaining,
    DynamicIndex& index, SparseExtendedLexicon& exlex, std::string staticpath, DocumentStore& docstore)
{
    if(termIDs.empty()) {
        return std::vector<unsigned int>();
    }

    std::priority_queue<ScorePair, std::vector<ScorePair>, greater_ScorePair> minheap;

    std::vector<query_primitive> listpointers;

    //Construct listpointers for each termID
    for(unsigned int i : termIDs) {
        listpointers.emplace_back(i, index, exlex, staticpath);
    }

    unsigned int did = 0;

    while(did < GlobalConst::UIntMax) {
        did = listpointers[0].nextGEQ(did);
        if(did == GlobalConst::UIntMax)
            break;

        unsigned int d = 0;
        for(size_t i = 1; i < listpointers.size() && (d = listpointers[i].nextGEQ(did)) == did; i++)
            ;

        if(d > did)
            did = d;
        else {
            std::vector<unsigned int> freqs;
            /* docID is in intersection; now get all frequencies */
            for(size_t i = 0; i < listpointers.size(); i++)
                freqs.push_back(listpointers[i].getFreq());

            /* compute BM25 score from frequencies and other data */
            //TODO: Allow other ranking functions here
            double score = BM25(freqs, docscontaining, docstore.getDocLength(did), docstore.getAverageDocLength(), docstore.getDocumentCount()); 

            if(minheap.size() < DAAT_SIZE) {
                minheap.emplace(did, score);
            }
            else if(score > minheap.top().score) {
                minheap.pop();
                minheap.emplace(did, score);
            }
            did++; /* and increase did to search for next post */
        }
    }

    std::vector<unsigned int> docs;
    docs.reserve(DAAT_SIZE);
    while(!minheap.empty()) {
        docs.push_back(minheap.top().docID);
        minheap.pop();
    }

    return docs;
}