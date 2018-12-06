#include "query_primitive.hpp"

#include "utility/util.hpp"

query_primitive::query_primitive(unsigned int termID, GlobalType::NonPosIndex& index, SparseExtendedLexicon& exlex, std::string staticpath) {
    lists.emplace_back(termID, index);

    std::vector<std::string> indexnames = Utility::readDirectory(staticpath);

    for(std::string& name : indexnames) {
        bool isZindex;
        if(name[0] == 'Z')
            isZindex = true;
        else if(name[0] == 'I')
            isZindex = false;
        else
            throw std::invalid_argument("Error, index name " + name + " is invalid");

        unsigned int indexnum = std::stoul(name.substr(1));

        try {
            lists.emplace_back(termID, staticpath + "/" + name, exlex.getNonPosLEQOffset(termID, indexnum, isZindex));
        }
        catch(const std::invalid_argument& e) {}
    }

    curdocIDs.resize(lists.size());
    docID = 0;
}

unsigned int query_primitive::nextGEQ(unsigned int x) {
    //Finds the QP with the smallest docID greater than x
    unsigned int min = GlobalConst::UIntMax;

    //Loop through every low level QP and call nextGEQ on it
    for(size_t i = 0; i < lists.size(); ++i) {
        bool failure = false;
        unsigned int next = lists[i].nextGEQ(x, failure);
        if(failure)
            curdocIDs[i] = GlobalConst::UIntMax;
        else {
            curdocIDs[i] = next;
            if(next <= min)
                min = next;
        }
    }
    docID = min;
    return min;
}

unsigned int query_primitive::getFreq() {
    //Make larger than any possible index num, specific number doesn't matter
    int minindexnum = lists.size();
    unsigned int freq = 0;

    //Loop through all the QP, find the pointer to the smallest index number, and get its frequency
    for(size_t i = 0; i < lists.size(); ++i) {
        if(curdocIDs[i] == docID) {
            int newindexnum = lists[i].getIndexNumber();

            if(newindexnum < minindexnum) {
                minindexnum = newindexnum;
                freq = lists[i].getFreq();
            }
        }
    }

    return freq;
}
