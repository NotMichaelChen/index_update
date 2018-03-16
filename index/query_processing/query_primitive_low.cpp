#include "query_primitive_low.hpp"

#include <algorithm>

#include "static_functions/postingIO.hpp"
#include "static_functions/compression_functions/varbyte.hpp"

query_primitive_low::query_primitive_low(unsigned int termID, GlobalType::NonPosIndex& index) {
    inmemory = true;
    postinglist = index[termID];
    postingindex = 0;
}

query_primitive_low::query_primitive_low(unsigned int termID, std::vector<mData>::iterator mdata) {
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

    docIDindex = 0;
    docblockpos = metadata->postings_blocks;
    blockindex = 0;
    freqdecompressed = false;
}

unsigned int query_primitive_low::nextGEQ(unsigned int pos, bool& failure) {
    //Reset to clear previous value
    failure = false;
    if(inmemory) {
        while(postingindex < postinglist.size() && postinglist[postingindex].docID < pos) {
            ++postingindex;
        }
        //Never go out of bounds, but notify upon return
        if(postingindex == postinglist.size()) {
            --postingindex;
            failure = true;
        }
        return postinglist[postingindex].docID;
    }
    else {
        size_t oldindex = docIDindex;
        //Find the correct block to look at
        while(docIDindex < last_docID.size() && last_docID[docIDindex] < pos) {
            //Move the docID block pointer
            docblockpos += blocksizes[(docIDindex*2)-1] + blocksizes[(docIDindex*2)];

            ++docIDindex;
        }
        //If it's different than our current block then decompress new block
        if(oldindex != docIDindex) {
            //Move the filestream to the new position
            ifile.seekg(docblockpos);
            size_t buffersize = blocksizes[docIDindex*2];
            docblock = read_block(buffersize, ifile, VBDecode, true);

            blockindex = 0;
            freqdecompressed = false;
        }
        //Perform standard docID searching
        while(blockindex < docblock.size() && docblock[blockindex] < pos)
            ++blockindex;

        if(blockindex == docblock.size()) {
            --blockindex;
            failure = true;
        }
        
        return docblock[blockindex];
    }
}

unsigned int query_primitive_low::getFreq() {
    if(inmemory) {
        return postinglist[postingindex].second;
    }
    else {
        if(!freqdecompressed) {
            size_t buffersize = blocksizes[(docIDindex*2)+1];
            freqblock = read_block(buffersize, ifile, VBDecode, false);
            freqdecompressed = true;
        }
        return freqblock[blockindex];
    }
}

int query_primitive_low::getIndexNumber() {
    if(inmemory)
        return -1;

    std::string filepath = metadata->filename;

    //WARNING: Uses unix-specific file separators
    std::string filename = std::string(std::find( filepath.rbegin(), filepath.rend(), '/' ).base(), filepath.end());

    return std::stoi(filename.substr(1, filename.length()));
}