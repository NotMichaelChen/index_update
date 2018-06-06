#include "postingIO.hpp"

#include <algorithm>

#include "global_parameters.hpp"
#include "compression_functions/varbyte.hpp"
#include "compression.hpp"

//Compresses and writes the given vector to the file
unsigned int write_block(std::vector<unsigned int>& block, std::ofstream& ofile, std::vector<uint8_t> encoder(unsigned int), bool delta) {
    std::vector<uint8_t> compressedblock = compress_block(block, encoder, delta);
    return write_raw_block(compressedblock, ofile);
}

//Reads and decompressed the block in the file
std::vector<unsigned int> read_block(size_t buffersize, std::ifstream& ifile, std::vector<unsigned int> decoder(std::vector<uint8_t>&), bool delta) {
    std::vector<uint8_t> unsignedbuffer = read_raw_block(buffersize, ifile);
    return decompress_block(unsignedbuffer, decoder, delta);
}

// Writes a vector of numbers byte by byte to the given file stream
// Returns how many bytes it wrote to the stream
unsigned int write_raw_block(std::vector<uint8_t>& num, std::ofstream& ofile) {
    /* Write the compressed posting to file byte by byte. */
    unsigned int start = ofile.tellp();
    for(auto it = num.begin(); it != num.end(); it++){
        ofile.write(reinterpret_cast<const char*>(&(*it)), sizeof(*it));
    }
    unsigned int end = ofile.tellp();
    return end - start;
}

// Reads a vector of unsigned chars from the given file stream
// Assumes input stream is already at the correct place
std::vector<uint8_t> read_raw_block(size_t buffersize, std::ifstream& ifile) {
    std::vector<char> buffer(buffersize);
    ifile.read(&buffer[0], buffersize);
    std::vector<uint8_t> unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
    return unsignedbuffer;
}

//Writes a posting list to disk with compression
template <typename T>
void write_postinglist(std::ofstream& ofile, unsigned int termID, std::vector<T>& postinglist, bool positional) {
    //initialize compression method, 1: varbyte
    //compression method for docID
    unsigned int doc_method = 1;
    //compression method for fragmentID (pos) or frequency (nonpos)
    unsigned int second_method = 1;
    //compression method for position
    unsigned int third_method = 1;

    //Construct compressed blocks of postings in memory
    std::vector<uint8_t> compressedblocks;
    std::vector<unsigned int> lastdocID;
    std::vector<unsigned int> compressedblocksizes;

    //Begin inclusive, End exclusive
    size_t blockbegin = 0;
    size_t blockend = BLOCKSIZE;

    while(blockend <= postinglist.size()) {
        std::vector<unsigned int> blockdocID;
        std::vector<unsigned int> blocksecond;
        std::vector<unsigned int> blockthird;

        //Grab blocksize number of postings
        for(auto postingiter = postinglist.begin() + blockbegin; postingiter != postinglist.begin() + blockend; postingiter++) {
            blockdocID.push_back(postingiter->docID);
            blocksecond.push_back(postingiter->second);
            if(positional) blockthird.push_back(postingiter->third);
        }

        //Compress the three vectors
        std::vector<uint8_t> compresseddocID = compress_block(blockdocID, VBEncode, true);
        std::vector<uint8_t> compressedsecond = compress_block(blocksecond, VBEncode, false);
        std::vector<uint8_t> compressedthird;
        if(positional) compressedthird = compress_block(blockthird, VBEncode, false);

        //Store the three vectors into the compressedblocks vector
        compressedblocks.insert(compressedblocks.end(), compresseddocID.begin(), compresseddocID.end());
        compressedblocks.insert(compressedblocks.end(), compressedsecond.begin(), compressedsecond.end());
        if(positional) compressedblocks.insert(compressedblocks.end(), compressedthird.begin(), compressedthird.end());

        //Store metadata
        compressedblocksizes.push_back(compresseddocID.size());
        compressedblocksizes.push_back(compressedsecond.size());
        if(positional) compressedblocksizes.push_back(compressedthird.size());
        lastdocID.push_back(blockdocID.back());

        blockbegin += BLOCKSIZE;
        blockend += BLOCKSIZE;
    }

    //Extra postings at end of block
    if(blockbegin != postinglist.size()) {
        std::vector<unsigned int> blockdocID;
        std::vector<unsigned int> blocksecond;
        std::vector<unsigned int> blockthird;

        for(auto postingiter = postinglist.begin() + blockbegin; postingiter != postinglist.end(); postingiter++) {
            blockdocID.push_back(postingiter->docID);
            blocksecond.push_back(postingiter->second);
            if(positional) blockthird.push_back(postingiter->third);
        }

        //Compress the three vectors
        std::vector<uint8_t> compresseddocID = compress_block(blockdocID, VBEncode, true);
        std::vector<uint8_t> compressedsecond = compress_block(blocksecond, VBEncode, false);
        std::vector<uint8_t> compressedthird;
        if(positional) compressedthird = compress_block(blockthird, VBEncode, false);

        //Store the three vectors into the compressedblocks vector
        compressedblocks.insert(compressedblocks.end(), compresseddocID.begin(), compresseddocID.end());
        compressedblocks.insert(compressedblocks.end(), compressedsecond.begin(), compressedsecond.end());
        if(positional) compressedblocks.insert(compressedblocks.end(), compressedthird.begin(), compressedthird.end());

        //Store metadata
        compressedblocksizes.push_back(compresseddocID.size());
        compressedblocksizes.push_back(compressedsecond.size());
        if(positional) compressedblocksizes.push_back(compressedthird.size());
        lastdocID.push_back(blockdocID.back());
    }

    //Compress the lastdocID and blocksize vectors
    std::vector<uint8_t> b_compressedblocksizes = compress_block(compressedblocksizes, VBEncode, false);
    std::vector<uint8_t> b_lastdocID = compress_block(lastdocID, VBEncode, false);

    //Compute total size of posting list in bytes
    //4 bytes per int * 8 plain ints = 32
    unsigned int lastdocIDlength = b_lastdocID.size();
    unsigned int blocksizeslength = b_compressedblocksizes.size();
    unsigned int blockslength = compressedblocks.size();
    unsigned int postinglistlen = postinglist.size();
    unsigned int totalbytes = 32 + b_lastdocID.size() + b_compressedblocksizes.size() + compressedblocks.size();
    //Add extra int for positional
    if(positional) totalbytes += 4;

    //Write out metadata
    //TODO: Compress metadata
    ofile.write(reinterpret_cast<const char *>(&termID), sizeof(termID));
    ofile.write(reinterpret_cast<const char *>(&totalbytes), sizeof(totalbytes));
    ofile.write(reinterpret_cast<const char *>(&postinglistlen), sizeof(postinglistlen));
    ofile.write(reinterpret_cast<const char *>(&doc_method), sizeof(doc_method));
    ofile.write(reinterpret_cast<const char *>(&second_method), sizeof(second_method));
    if(positional) ofile.write(reinterpret_cast<const char *>(&third_method), sizeof(third_method));

    //Write out metadata and compressed postings
    ofile.write(reinterpret_cast<const char *>(&lastdocIDlength), sizeof(lastdocIDlength));
    write_raw_block(b_lastdocID, ofile);
    ofile.write(reinterpret_cast<const char *>(&blocksizeslength), sizeof(blocksizeslength));
    write_raw_block(b_compressedblocksizes, ofile);
    ofile.write(reinterpret_cast<const char *>(&blockslength), sizeof(blockslength));
    write_raw_block(compressedblocks, ofile);
}

//Given an ifstream, read the positional posting list indicated by the metadata
std::vector<Posting> read_pos_postinglist(std::ifstream& ifile, unsigned int termID) {
    //Don't read termID since it is already read and is given to us
    unsigned int totalbytes;
    ifile.read(reinterpret_cast<char *>(&totalbytes), sizeof(totalbytes));
    if(totalbytes <= 36)
        throw std::runtime_error("Error, invalid posting list size in static block");

    std::vector<Posting> postinglist;
    
    //Read in posting list into memory
    //Skip termID and totalbytes ints
    std::vector<uint8_t> byteslist = read_raw_block(totalbytes-8, ifile);

    //Skip beginning metadata
    size_t blockptr = 16;

    //Skip lastdocID
    unsigned int lastdocIDlen;
    std::memcpy(&lastdocIDlen, byteslist.data() + blockptr, sizeof(lastdocIDlen));
    blockptr += lastdocIDlen + 4;

    //Read blocksizes into separate vector
    unsigned int blocksizeslength;
    std::memcpy(&blocksizeslength, byteslist.data() + blockptr, sizeof(blocksizeslength));
    blockptr += 4;
    std::vector<uint8_t> b_blocksizes(byteslist.begin() + blockptr, byteslist.begin() + blockptr + blocksizeslength);

    //Decompress block vector
    std::vector<unsigned int> blocksizes = decompress_block(b_blocksizes, VBDecode, false);
    blockptr += blocksizeslength;

    if(blocksizes.size() % 3 != 0) {
        throw std::invalid_argument("Error, blocksize array is not a multiple of 3: " + std::to_string(blocksizes.size()));
    }

    //Skip blocks int
    blockptr += 4;

    //For every three blocksize entries, read in three blocks of numbers and insert postings into the index
    for(size_t i = 0; i < blocksizes.size(); i += 3) {
        unsigned int doclength = blocksizes[i];
        unsigned int secondlength = blocksizes[i+1];
        unsigned int thirdlength = blocksizes[i+2];
    
        std::vector<unsigned int> docIDs, secondvec, thirdvec;

        std::vector<uint8_t> b_docIDs(byteslist.begin() + blockptr, byteslist.begin() + blockptr + doclength);
        std::vector<uint8_t> b_second(byteslist.begin() + blockptr, byteslist.begin() + blockptr + secondlength);
        std::vector<uint8_t> b_third(byteslist.begin() + blockptr, byteslist.begin() + blockptr + thirdlength);

        docIDs = decompress_block(b_docIDs, VBDecode, true);
        secondvec = decompress_block(b_second, VBDecode, false);
        thirdvec = decompress_block(b_third, VBDecode, false);

        if(docIDs.size() != secondvec.size() || secondvec.size() != thirdvec.size()) {
            throw std::invalid_argument("Error, vectors mismatched in size while reading index: " + std::to_string(docIDs.size()) + "," + std::to_string(secondvec.size()) + "," + std::to_string(thirdvec.size()));
        }

        if(!std::is_sorted(docIDs.begin(), docIDs.end())) {
            throw std::invalid_argument("Error, docID array not sorted");
        }

        for(size_t j = 0; j < docIDs.size(); j++) {
            Posting newpost;
            newpost.termID = termID;
            newpost.docID = docIDs[j];
            newpost.second = secondvec[j];
            newpost.third = thirdvec[j];

            postinglist.push_back(newpost);
        }
    }
    return postinglist;
}

//Given an ifstream, read the nonpositional posting list indicated by the metadata
std::vector<nPosting> read_nonpos_postinglist(std::ifstream& ifile, unsigned int termID) {
    //Don't read termID since it is already read and is given to us
    unsigned int totalbytes;
    ifile.read(reinterpret_cast<char *>(&totalbytes), sizeof(totalbytes));
    if(totalbytes <= 32)
        throw std::runtime_error("Error, invalid posting list size in static block");

    std::vector<nPosting> postinglist;

    //Read in posting list into memory
    //Skip termID and totalbytes ints
    std::vector<uint8_t> byteslist = read_raw_block(totalbytes-8, ifile);

    //Skip beginning metadata
    size_t blockptr = 12;

    //Skip lastdocID
    unsigned int lastdocIDlen;
    std::memcpy(&lastdocIDlen, byteslist.data() + blockptr, sizeof(lastdocIDlen));
    blockptr += lastdocIDlen + 4;

    //Read blocksizes into separate vector
    unsigned int blocksizeslength;
    std::memcpy(&blocksizeslength, byteslist.data() + blockptr, sizeof(blocksizeslength));
    blockptr += 4;
    std::vector<uint8_t> b_blocksizes(byteslist.begin() + blockptr, byteslist.begin() + blockptr + blocksizeslength);

    //Decompress block vector
    std::vector<unsigned int> blocksizes = decompress_block(b_blocksizes, VBDecode, false);
    blockptr += blocksizeslength;

    if(blocksizes.size() % 2 != 0) {
        throw std::invalid_argument("Error, blocksize array is not a multiple of 2: " + std::to_string(blocksizes.size()));
    }

    //Skip blocks int
    blockptr += 4;

    //For every two blocksize entries, read in three blocks of numbers and insert postings into the index
    for(size_t i = 0; i < blocksizes.size(); i += 2) {
        unsigned int doclength = blocksizes[i];
        unsigned int secondlength = blocksizes[i+1];
    
        std::vector<unsigned int> docIDs, secondvec;

        std::vector<uint8_t> b_docIDs(byteslist.begin() + blockptr, byteslist.begin() + blockptr + doclength);
        std::vector<uint8_t> b_second(byteslist.begin() + blockptr, byteslist.begin() + blockptr + secondlength);

        docIDs = decompress_block(b_docIDs, VBDecode, true);
        secondvec = decompress_block(b_second, VBDecode, false);

        if(docIDs.size() != secondvec.size()) {
            throw std::invalid_argument("Error, vectors mismatched in size while reading index: " + std::to_string(docIDs.size()) + "," + std::to_string(secondvec.size()));
        }

        if(!std::is_sorted(docIDs.begin(), docIDs.end())) {
            throw std::invalid_argument("Error, docID array not sorted");
        }

        for(size_t j = 0; j < docIDs.size(); j++) {
            nPosting newpost;
            newpost.termID = termID;
            newpost.docID = docIDs[j];
            newpost.second = secondvec[j];

            postinglist.push_back(newpost);
        }
    }

    return postinglist;
}

//Explicitly instantiate templates for write_postinglist
template void write_postinglist<Posting>(std::ofstream& ofile, unsigned int termID, std::vector<Posting>& postinglist, bool positional);
template void write_postinglist<nPosting>(std::ofstream& ofile, unsigned int termID, std::vector<nPosting>& postinglist, bool positional);