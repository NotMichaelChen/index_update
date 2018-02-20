#include "query_primitives.hpp"

#include "../static_functions/postingIO.hpp"
#include "../static_functions/compression_functions/varbyte.hpp"

query_primitive::query_primitive(int termID, std::map<unsigned int, std::vector<nPosting>>& index) {
    inmemory = true;
    postinglist = index[termID];
    postingindex = 0;
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

    //Decompress and store the first docID
    //Only decompress frequency block if getFreq is called
    buffersize = blocksizes[0];
    docblock = read_block(buffersize, ifile, VBDecode, true);

    docIDindex = 1;
}

unsigned int query_primitive::nextGEQ(unsigned int pos) {
    if(inmemory) {
        while(postingindex < postinglist.size() && postinglist[postingindex].docID < pos) {
            ++postingindex;
        }
        //Never go out of bounds
        //TODO: determine what to return if out of bounds
        if(postingindex == postinglist.size())
            --postingindex;
        return postinglist[postingindex].docID;
    }
    else {
        size_t oldindex = docIDindex;
        //Find the correct block to look at
        while(docIDindex < last_docID.size() && last_docID[docIDindex] < pos) {
            //Seek to next block location
            long amount = blocksizes[(docIDindex*2)-1] + blocksizes[(docIDindex*2)];
            ifile.seekg(amount, std::ios_base::cur);

            ++docIDindex;
        }
        //If it's different than our current block then decompress new block
        if(oldindex != docIDindex) {
            size_t buffersize = blocksizes[docIDindex*2];
            docblock = read_block(buffersize, ifile, VBDecode, true);

            blockindex = 0;
        }

        //Perform standard docID searching
        while(blockindex < docblock.size() && docblock[blockindex] < pos)
            ++blockindex;
        
        return docblock[blockindex];
    }
}