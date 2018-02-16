#include "query_primitives.hpp"

query_primitive::query_primitive(int termID, std::map<unsigned int, std::vector<nPosting>>& index) {
    inmemory = true;
    postinglist = index[termID];
}

query_primitive::query_primitive(int termID, std::vector<mData>::iterator mdata) {
    inmemory = false;
    metadata = mdata;

    ifile.open(metadata->filename);
    ifile.seekg(metadata->last_docID);
    
}