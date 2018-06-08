#ifndef QUERY_PRIMITIVE_HPP
#define QUERY_PRIMITIVE_HPP

#include <map>
#include <vector>

#include "posting.hpp"
#include "extended_lexicon.hpp"
#include "sparse_lexicon.hpp"
#include "query_primitive_low.hpp"

class query_primitive {
public:
    query_primitive(unsigned int termID, GlobalType::NonPosIndex& index, SparseExtendedLexicon& exlex, std::string staticpath);

    //Advances QP to next docID greater than x
    unsigned int nextGEQ(unsigned int x);
    //Return frequency of the docID that the QP is pointing to
    unsigned int getFreq();
private:
    std::vector<query_primitive_low> lists;
    //Contains the current docID that each QP is pointed at
    std::vector<unsigned int> curdocIDs;
    //Contains the minimum docID of all QPs
    unsigned int docID;
};

#endif