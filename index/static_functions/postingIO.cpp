#include "postingIO.hpp"

#include <algorithm>

#include "../global_parameters.hpp"
#include "../varbyte.hpp"
#include "compression.hpp"

// Writes a vector of numbers byte by byte to the given file stream
// Returns how many bytes it wrote to the stream
template <typename T>
unsigned int write_block(std::vector<T>& num, std::ofstream& ofile){
    /* Write the compressed posting to file byte by byte. */
    unsigned int start = ofile.tellp();
    for(typename std::vector<T>::iterator it = num.begin(); it != num.end(); it++){
        ofile.write(reinterpret_cast<const char*>(&(*it)), sizeof(*it));
    }
    unsigned int end = ofile.tellp();
    return end - start;
}

// Reads a vector of unsigned chars from the given file stream
// Assumes input stream is already at the correct place
std::vector<uint8_t> read_block(size_t buffersize, std::ifstream& ifile) {
    std::vector<char> buffer(buffersize);
    ifile.read(&buffer[0], buffersize);
    std::vector<uint8_t> unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
    return unsignedbuffer;
}

//Writes a posting list to disk with compression
template <typename T>
void write_postinglist(std::ofstream& ofile, std::string& filepath, unsigned int termID, std::vector<T>& postinglist, ExtendedLexicon& exlex, bool positional) {
    //initialize compression method, 1: varbyte
    //compression method for docID
    unsigned int doc_method = 1;
    //compression method for fragmentID (pos) or frequency (nonpos)
    unsigned int second_method = 1;
    //compression method for position
    unsigned int third_method = 1;

    mData metadata;
    metadata.filename = filepath;
    metadata.start_pos = ofile.tellp();

    //Write out metadata
    //TODO: Compress metadata
    //required to get address of variable
    size_t postinglistsize = postinglist.size();
    ofile.write(reinterpret_cast<const char *>(&termID), sizeof(termID));
    ofile.write(reinterpret_cast<const char *>(&(postinglistsize)), sizeof(postinglistsize));
    ofile.write(reinterpret_cast<const char *>(&doc_method), sizeof(doc_method));
    ofile.write(reinterpret_cast<const char *>(&second_method), sizeof(second_method));
    if(positional) ofile.write(reinterpret_cast<const char *>(&third_method), sizeof(third_method));

    //Construct compressed blocks of postings in memory
    std::vector<std::vector<uint8_t>> compressedblocks;
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
        compressedblocks.push_back(compresseddocID);
        compressedblocks.push_back(compressedsecond);
        if(positional) compressedblocks.push_back(compressedthird);

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
        compressedblocks.push_back(compresseddocID);
        compressedblocks.push_back(compressedsecond);
        if(positional) compressedblocks.push_back(compressedthird);

        //Store metadata
        compressedblocksizes.push_back(compresseddocID.size());
        compressedblocksizes.push_back(compressedsecond.size());
        if(positional) compressedblocksizes.push_back(compressedthird.size());
        lastdocID.push_back(blockdocID.back());
    }

    //Write out metadata and compressed postings
    metadata.last_docID = ofile.tellp();
    std::vector<uint8_t> compressedlastdocID = compress_block(lastdocID, VBEncode, false);
    write_block<uint8_t>(compressedlastdocID, ofile);
    metadata.blocksizes = ofile.tellp();
    std::vector<uint8_t> encodedblocksizes = compress_block(compressedblocksizes, VBEncode, false);
    write_block<uint8_t>(encodedblocksizes, ofile);
    metadata.postings_blocks = ofile.tellp();

    for(auto iter = compressedblocks.begin(); iter != compressedblocks.end(); iter++) {
        write_block<uint8_t>(*iter, ofile);
    }

    metadata.end_offset = ofile.tellp();
    if(positional) exlex.addPositional(termID, metadata);
    else exlex.addNonPositional(termID, metadata);
}

//Given an ifstream, read the positional posting list indicated by the metadata
std::vector<Posting> read_pos_postinglist(std::ifstream& ifile, std::vector<mData>::iterator metadata, unsigned int termID) {
    std::vector<Posting> postinglist;
    size_t postinglistlength;
    unsigned int doc_method, second_method, third_method;
    
    //Don't read termID since it is already read and is given to us
    ifile.read(reinterpret_cast<char *>(&postinglistlength), sizeof(postinglistlength));
    ifile.read(reinterpret_cast<char *>(&doc_method), sizeof(doc_method));
    ifile.read(reinterpret_cast<char *>(&second_method), sizeof(second_method));
    ifile.read(reinterpret_cast<char *>(&third_method), sizeof(third_method));

    postinglist.reserve(postinglistlength);

    //Skip the lastdocID block since we don't need it for reading
    ifile.seekg(metadata->blocksizes);

    unsigned int buffersize = metadata->postings_blocks - metadata->blocksizes;
    std::vector<uint8_t> unsignedbuffer = read_block(buffersize, ifile);
    std::vector<unsigned int> blocksizes = decompress_block(unsignedbuffer, VBDecode, false);
    unsignedbuffer.clear();

    if(blocksizes.size() % 3 != 0) {
        throw std::invalid_argument("Error, blocksize array is not a multiple of 3: " + std::to_string(blocksizes.size()));
    }

    //For every three blocksize entries, read in three blocks of numbers and insert postings into the index
    for(size_t i = 0; i < blocksizes.size(); i += 3) {
        unsigned int doclength = blocksizes[i];
        unsigned int secondlength = blocksizes[i+1];
        unsigned int thirdlength = blocksizes[i+2];
    
        std::vector<unsigned int> docIDs, secondvec, thirdvec;

        unsignedbuffer = read_block(doclength, ifile);
        docIDs = decompress_block(unsignedbuffer, VBDecode, true);
        unsignedbuffer.clear();

        unsignedbuffer = read_block(secondlength, ifile);
        secondvec = decompress_block(unsignedbuffer, VBDecode, false);
        unsignedbuffer.clear();

        unsignedbuffer = read_block(thirdlength, ifile);
        thirdvec = decompress_block(unsignedbuffer, VBDecode, false);
        unsignedbuffer.clear();

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

    if(postinglist.size() != postinglistlength) {
        throw std::invalid_argument("Error, posting list length not equal to specified length: " + std::to_string(postinglist.size()) + "," + std::to_string(postinglistlength));
    }

    return postinglist;
}

//Given an ifstream, read the nonpositional posting list indicated by the metadata
std::vector<nPosting> read_nonpos_postinglist(std::ifstream& ifile, std::vector<mData>::iterator metadata, unsigned int termID) {
    std::vector<nPosting> postinglist;
    size_t postinglistlength;
    unsigned int doc_method, second_method;
    
    //Don't read termID since it is already read and is given to us
    ifile.read(reinterpret_cast<char *>(&postinglistlength), sizeof(postinglistlength));
    ifile.read(reinterpret_cast<char *>(&doc_method), sizeof(doc_method));
    ifile.read(reinterpret_cast<char *>(&second_method), sizeof(second_method));

    postinglist.reserve(postinglistlength);

    //Skip the lastdocID block since we don't need it for reading
    ifile.seekg(metadata->blocksizes);

    unsigned int buffersize = metadata->postings_blocks - metadata->blocksizes;
    std::vector<uint8_t> unsignedbuffer = read_block(buffersize, ifile);
    std::vector<unsigned int> blocksizes = decompress_block(unsignedbuffer, VBDecode, false);
    unsignedbuffer.clear();

    if(blocksizes.size() % 2 != 0) {
        throw std::invalid_argument("Error, blocksize array is not a multiple of 2: " + std::to_string(blocksizes.size()));
    }

    //For every two blocksize entries, read in three blocks of numbers and insert postings into the index
    for(size_t i = 0; i < blocksizes.size(); i += 2) {
        unsigned int doclength = blocksizes[i];
        unsigned int secondlength = blocksizes[i+1];
    
        std::vector<unsigned int> docIDs, secondvec;

        unsignedbuffer = read_block(doclength, ifile);
        docIDs = decompress_block(unsignedbuffer, VBDecode, true);
        unsignedbuffer.clear();

        unsignedbuffer = read_block(secondlength, ifile);
        secondvec = decompress_block(unsignedbuffer, VBDecode, false);
        unsignedbuffer.clear();

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

    if(postinglist.size() != postinglistlength) {
        throw std::invalid_argument("Error, posting list length not equal to specified length: " + std::to_string(postinglist.size()) + "," + std::to_string(postinglistlength));
    }

    return postinglist;
}

//Explicitly instantiate templates for write_postinglist
template void write_postinglist<Posting>(std::ofstream& ofile, std::string& filepath, unsigned int termID, std::vector<Posting>& postinglist, ExtendedLexicon& exlex, bool positional);
template void write_postinglist<nPosting>(std::ofstream& ofile, std::string& filepath, unsigned int termID, std::vector<nPosting>& postinglist, ExtendedLexicon& exlex, bool positional);