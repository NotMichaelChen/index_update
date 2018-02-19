#include "query_primitives.hpp"

#include "../static_functions/postingIO.hpp"
#include "../static_functions/compression_functions/varbyte.hpp"

query_primitive::query_primitive(int termID, std::map<unsigned int, std::vector<nPosting>>& index) {
    inmemory = true;
    postinglist = index[termID];
}

query_primitive::query_primitive(int termID, std::vector<mData>::iterator mdata) {
    inmemory = false;
    metadata = mdata;

    ifile.open(metadata->filename);
    //TODO: Read static metadata and use it
    ifile.seekg(metadata->last_docID);

    //Save the last_docID array in memory
    size_t buffersize = metadata->blocksizes - metadata->last_docID;
    last_docID = read_block(buffersize, ifile, VBDecode, false);

    //Get blocksizes array
    buffersize = metadata->postings_blocks - metadata->blocksizes;
    blocksizes = read_block(buffersize, ifile, VBDecode, false);

    //Decompress and store the first docID and frequency block
    buffersize = blocksizes[0];
    docblock = read_block(buffersize, ifile, VBDecode, true);

    buffersize = blocksizes[1];
    freqblock = read_block(buffersize, ifile, VBDecode, false);

    blockindex = 2;
}