#include "query_primitive_low.hpp"

#include <algorithm>

#include "static_functions/postingIO.hpp"
#include "static_functions/bytesIO.hpp"
#include "static_functions/compression_functions/varbyte.hpp"

query_primitive_low::query_primitive_low(unsigned int termID, DynamicIndex& index) {
    inmemory = true;
    //Do *not* assume that in-memory posting lists are sorted
    postinglist = index.getNPostingList(termID);
    std::sort(postinglist->begin(), postinglist->end());
    postingindex = 0;
}

//filepath: The path to the index that the QPL points to
//LEQpos: Pointer to the closest termID that is less than or equal to the desired termID. May be greater than termID if termID
//          is smaller than all termIDs in the block
query_primitive_low::query_primitive_low(unsigned int termID, std::string path, size_t LEQpos) {
    inmemory = false;
    filepath = path;

    //Can assume that static posting lists are sorted

    //Determine if term exists in index
    //TODO: Should be abstracted into sparse lexicon
    ifile.open(filepath);
    if(!ifile)
        throw std::invalid_argument("Error: invalid filepath in query_primitive_low: " + filepath);

    ifile.seekg(LEQpos);

    unsigned int disktermID;
    readFromBytes(disktermID, ifile);
    unsigned int postinglistlength;

    while(ifile && disktermID < termID) {
        readFromBytes(postinglistlength, ifile);
        ifile.seekg(postinglistlength-8, std::ios_base::cur);

        readFromBytes(disktermID, ifile);
    }

    if(disktermID != termID) {
        throw std::invalid_argument("Error, term does not exist in index");
    }

    readFromBytes(postinglistlength, ifile);
    //Skip postings count and compression methods
    //WARNING: Assumed non-positional postings here
    ifile.seekg(12, std::ios_base::cur);

    //Save last_docID array in memory
    unsigned int lastdocIDlen;
    readFromBytes(lastdocIDlen, ifile);
    last_docID = read_block(lastdocIDlen, ifile, VBDecode, false);

    //Get blocksizes array
    unsigned int blocksizeslen;
    readFromBytes(blocksizeslen, ifile);
    blocksizes = read_block(blocksizeslen, ifile, VBDecode, false);

    //Skip postingblockssize var
    ifile.seekg(4, std::ios_base::cur);

    //Store some metadata
    docIDindex = 0;
    docblockpos = ifile.tellg();
    blockindex = 0;
    freqdecompressed = false;

    //Decompress and store the first docID
    //Only decompress frequency block if getFreq is called
    size_t buffersize = blocksizes[0];
    docblock = read_block(buffersize, ifile, VBDecode, true);
}

unsigned int query_primitive_low::nextGEQ(unsigned int docID, bool& failure) {
    //Reset to clear previous value
    failure = false;
    if(inmemory) {
        while(postingindex < postinglist->size() && (*postinglist)[postingindex].docID < docID) {
            ++postingindex;
        }
        //Notify failure upon return
        if(postingindex == postinglist->size()) {
            failure = true;
            return GlobalConst::UIntMax;
        }
        return (*postinglist)[postingindex].docID;
    }
    else {
        size_t oldindex = docIDindex;
        //Find the correct block to look at
        while(docIDindex < last_docID.size() && last_docID[docIDindex] < docID) {
            //Move the docID block pointer
            docblockpos += blocksizes[(docIDindex*2)] + blocksizes[(docIDindex*2)+1];

            ++docIDindex;
        }
        if(docIDindex == last_docID.size()) {
            failure = true;
            return GlobalConst::UIntMax;
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
        while(blockindex < docblock.size() && docblock[blockindex] < docID)
            ++blockindex;

        if(blockindex == docblock.size()) {
            failure = true;
            return GlobalConst::UIntMax;
        }

        return docblock[blockindex];
    }
}

//Assumes query primitive is in valid state
//Undefined if nextGEQ returned invalid
unsigned int query_primitive_low::getFreq() {
    if(inmemory) {
        return (*postinglist)[postingindex].second;
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

    //WARNING: Uses unix-specific file separators
    std::string filename = std::string(std::find( filepath.rbegin(), filepath.rend(), '/' ).base(), filepath.end());

    return std::stoi(filename.substr(1, filename.length()));
}