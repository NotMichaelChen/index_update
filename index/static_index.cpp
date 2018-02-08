#include "static_index.hpp"

#include <iostream>
#include <functional>
#include <dirent.h>

#include "varbyte.hpp"

//List all the files in a directory
//Defined locally only for the static_index methods
std::vector<std::string> read_directory( std::string path ){
    std::vector <std::string> result;
    dirent* de;
    DIR* dp;
    errno = 0;
    dp = opendir( path.empty() ? "." : path.c_str() );
    if (dp){
            while (true){
                errno = 0;
                de = readdir( dp );
                if (de == NULL) break;
                std::string d_n(de->d_name);
                result.push_back( d_n );
            }
            closedir( dp );
        }
    return result;
}

StaticIndex::StaticIndex(std::string dir, int blocksize) : indexdir(dir), blocksize(blocksize), exlex() {
    posdir = "./" + dir + "/positional/";
    nonposdir = "./" + dir + "/non_positional/";
}

//Writes the positional index to disk, which means it is saved either in file Z0 or I0.
void StaticIndex::write_p_disk(Pos_Map_Iter indexbegin, Pos_Map_Iter indexend) {
    std::string filename = posdir;
    //Z0 exists
    if(std::ifstream(filename + "Z0"))
        filename += "I0";
    else
        filename += "Z0";

    std::ofstream ofile(filename);

    if (ofile.is_open()){
        auto vit = indexbegin->second.begin();
        auto vend = indexbegin->second.end();
        write_index<Pos_Map_Iter>(filename, ofile, true, indexbegin, indexend);

        ofile.close();
    }else{
        std::cerr << "File cannot be opened." << std::endl;
    }

    merge_test(true);
}

//Writes the non-positional index to disk, which is saved in either file Z0 or I0
void StaticIndex::write_np_disk(NonPos_Map_Iter indexbegin, NonPos_Map_Iter indexend) {
    std::string filename = nonposdir;
    //Z0 exists
    if(std::ifstream(filename + "Z0"))
        filename += "I0";
    else
        filename += "Z0";

    std::ofstream ofile(filename);

    if (ofile.is_open()){
        auto vit = indexbegin->second.begin();
        auto vend = indexbegin->second.end();
        write_index<NonPos_Map_Iter>(filename, ofile, false, indexbegin, indexend);

        ofile.close();
    }else{
        std::cerr << "File cannot be opened." << std::endl;
    }

    merge_test(false);
}

// Writes a vector of numbers byte by byte to the given file stream
// Returns how many bytes it wrote to the stream
template <typename T>
unsigned int StaticIndex::write_block(std::vector<T>& num, std::ofstream& ofile){
    /* Write the compressed posting to file byte by byte. */
    unsigned int start = ofile.tellp();
    for(typename std::vector<T>::iterator it = num.begin(); it != num.end(); it++){
        ofile.write(reinterpret_cast<const char*>(&(*it)), sizeof(*it));
    }
    unsigned int end = ofile.tellp();
    return end - start;
}

//Compresses a vector of posting data using the given compression method
//When delta encoding, assume field is already sorted
std::vector<uint8_t> StaticIndex::compress_block(std::vector<unsigned int>& field, std::vector<uint8_t> encoder(std::vector<unsigned int>&), bool delta) {
    std::vector<uint8_t> compressed;
    if(delta) {
        std::vector<unsigned int> deltaencode;
        deltaencode.push_back(field[0]);

        for(size_t i = 1; i < field.size(); i++) {
            if(field[i] < field[i-1])
                std::cerr << "negative during delta compressing " << field[i-1] << ' ' << field[i] << "\n";
            deltaencode.push_back(field[i] - field[i-1]);
        }
        compressed = encoder(deltaencode);
    }
    else {
        compressed = encoder(field);
    }
    return compressed;
}

//Decompresses a vector of posting data using the given decompression method
std::vector<unsigned int> StaticIndex::decompress_block(std::vector<uint8_t>& block, std::vector<unsigned int> decoder(std::vector<uint8_t>&), bool delta) {
    std::vector<unsigned int> decompressed;
    decompressed = decoder(block);
    if(decompressed.size() == 0) {
        throw std::invalid_argument("Error, decompress_block final size is zero");
    }
    if(delta) {
        std::vector<unsigned int> undelta(decompressed.size());
        undelta[0] = decompressed[0];

        for(size_t i = 1; i < decompressed.size(); i++) {
            undelta[i] = undelta[i-1] + decompressed[i];
        }
        return undelta;
    }
    return decompressed;
}

//Writes an inverted index to disk using compressed postings
//filepath: The filename of the file being written to
//          INCLUDES THE PATH
template <typename T>
void StaticIndex::write_index(std::string& filepath, std::ofstream& ofile, bool positional, T indexbegin, T indexend) {
    //initialize compression method, 1: varbyte
    //compression method for docID
    unsigned int doc_method = 1;
    //compression method for fragmentID (pos) or frequency (nonpos)
    unsigned int second_method = 1;
    //compression method for position
    unsigned int third_method = 1;

    //for each posting list in the index
    for(auto postinglistiter = indexbegin; postinglistiter != indexend; postinglistiter++) {
        mData metadata;
        metadata.filename = filepath;
        metadata.start_pos = ofile.tellp();

        unsigned int termID = postinglistiter->first;

        //Write out metadata
        //TODO: Compress metadata

        size_t postinglistsize = postinglistiter->second.size();
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
        size_t blockend = blocksize;

        while(blockend <= postinglistsize) {
            std::vector<unsigned int> blockdocID;
            std::vector<unsigned int> blocksecond;
            std::vector<unsigned int> blockthird;

            //Grab blocksize number of postings
            for(auto postingiter = postinglistiter->second.begin() + blockbegin; postingiter != postinglistiter->second.begin() + blockend; postingiter++) {
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

            blockbegin += blocksize;
            blockend += blocksize;
        }

        //Extra postings at end of block
        if(blockbegin != postinglistsize) {
            std::vector<unsigned int> blockdocID;
            std::vector<unsigned int> blocksecond;
            std::vector<unsigned int> blockthird;

            for(auto postingiter = postinglistiter->second.begin() + blockbegin; postingiter != postinglistiter->second.end(); postingiter++) {
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
}

//Writes a posting list to disk with compression
template <typename T>
void StaticIndex::write_postinglist(std::ofstream& ofile, std::string& filepath, unsigned int termID, std::vector<T>& postinglist, bool positional) {
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
    size_t blockend = blocksize;

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

        blockbegin += blocksize;
        blockend += blocksize;
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
std::vector<Posting> StaticIndex::read_pos_postinglist(std::ifstream& ifile, std::vector<mData>::iterator metadata, unsigned int termID) {
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
    std::vector<char> buffer(buffersize);
    ifile.read(&buffer[0], buffersize);
    std::vector<uint8_t> unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
    std::vector<unsigned int> blocksizes = VBDecode(unsignedbuffer);
    buffer.clear();
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

        buffer.resize(doclength);
        ifile.read(&buffer[0], doclength);
        unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
        docIDs = decompress_block(unsignedbuffer, VBDecode, true);
        buffer.clear();
        unsignedbuffer.clear();

        buffer.resize(secondlength);
        ifile.read(&buffer[0], secondlength);
        unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
        secondvec = decompress_block(unsignedbuffer, VBDecode, false);
        buffer.clear();
        unsignedbuffer.clear();

        buffer.resize(thirdlength);
        ifile.read(&buffer[0], thirdlength);
        unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
        thirdvec = decompress_block(unsignedbuffer, VBDecode, false);
        buffer.clear();
        unsignedbuffer.clear();

        if(docIDs.size() != secondvec.size() || secondvec.size() != thirdvec.size()) {
            throw std::invalid_argument("Error, vectors mismatched in size while reading index: " + std::to_string(docIDs.size()) + "," + std::to_string(secondvec.size()) + "," + std::to_string(thirdvec.size()));
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
std::vector<nPosting> StaticIndex::read_nonpos_postinglist(std::ifstream& ifile, std::vector<mData>::iterator metadata, unsigned int termID) {
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
    std::vector<char> buffer(buffersize);
    ifile.read(&buffer[0], buffersize);
    std::vector<uint8_t> unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
    std::vector<unsigned int> blocksizes = VBDecode(unsignedbuffer);
    buffer.clear();
    unsignedbuffer.clear();

    if(blocksizes.size() % 2 != 0) {
        throw std::invalid_argument("Error, blocksize array is not a multiple of 2: " + std::to_string(blocksizes.size()));
    }

    //For every two blocksize entries, read in three blocks of numbers and insert postings into the index
    for(size_t i = 0; i < blocksizes.size(); i += 2) {
        unsigned int doclength = blocksizes[i];
        unsigned int secondlength = blocksizes[i+1];
    
        std::vector<unsigned int> docIDs, secondvec;

        buffer.resize(doclength);
        ifile.read(&buffer[0], doclength);
        unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
        docIDs = decompress_block(unsignedbuffer, VBDecode, true);
        buffer.clear();
        unsignedbuffer.clear();

        buffer.resize(secondlength);
        ifile.read(&buffer[0], secondlength);
        unsignedbuffer = std::vector<uint8_t>(buffer.begin(), buffer.end());
        secondvec = decompress_block(unsignedbuffer, VBDecode, false);
        buffer.clear();
        unsignedbuffer.clear();

        if(docIDs.size() != secondvec.size()) {
            throw std::invalid_argument("Error, vectors mismatched in size while reading index: " + std::to_string(docIDs.size()) + "," + std::to_string(secondvec.size()));
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

/**
 * Test if there are two files of same index number on disk.
 * If there is, merge them and then call merge_test again until
 * all index numbers have only one file each.
 * Assumes that only one index is ever written to disk
 */
void StaticIndex::merge_test(bool isPositional) {
    //Assign directory the correct string based on the parameter
    std::string directory = isPositional ? posdir : nonposdir;

    std::vector<std::string> files = read_directory(directory);
    auto dir_iter = files.begin();

    while(dir_iter != files.end()) {
        //If any index file starts with an 'I', then we need to merge it
        if(dir_iter->size() > 1 && (*dir_iter)[0] == 'I') {
            //Get the number of the index
            int indexnum = std::stoi(dir_iter->substr(1));
            merge(indexnum, isPositional);

            files.clear();
            files = read_directory(directory);
            dir_iter = files.begin();
        }
        else {
            dir_iter++;
        }
    }
}

/**
 * Merges the indexes of the given order. Both the Z-index and I-index must already exist before
 * this method is called.
 */
void StaticIndex::merge(int indexnum, bool positional) {
    std::ifstream zfilestream;
    std::ifstream ifilestream;
    std::ofstream ofile;
    std::string dir = positional ? posdir : nonposdir;

    //determine the name of the output file, if "Z" file exists, than compressed to "I" file.
    zfilestream.open(dir + "Z" + std::to_string(indexnum));
    ifilestream.open(dir + "I" + std::to_string(indexnum));

    std::string namebase1 = "Z" + std::to_string(indexnum);
    std::string namebase2 = "I" + std::to_string(indexnum);
    std::string namebaseo;

    char flag = 'Z';
    std::ifstream filetest(dir + "Z" + std::to_string(indexnum+1));
    if(filetest.good())
        flag = 'I';
    filetest.close();

    namebaseo = flag + std::to_string(indexnum + 1);
    ofile.open(dir + namebaseo);

    //declare variables for loop
    std::vector<mData>::iterator metai, metaz;

    unsigned int ZtermID, ItermID;
    zfilestream.read(reinterpret_cast<char *>(&ZtermID), sizeof(ZtermID));
    ifilestream.read(reinterpret_cast<char *>(&ItermID), sizeof(ItermID));
    while(true) {
        if(ItermID < ZtermID) {
            if( positional ) metai = exlex.getPositional(ItermID, dir+namebase2);
            else metai = exlex.getNonPositional(ItermID, dir+namebase2);
            //Calculate shift to use for updating the metadata
            long shift = ofile.tellp() - metai->start_pos;
            //Subtract four since we already read the termID
            int length = metai->end_offset - metai->start_pos - sizeof(unsigned int);
            char* buffer = new char [length];
            ifilestream.read(buffer, length);
            //Write termID then rest of posting list
            ofile.write(reinterpret_cast<const char *>(&ItermID), sizeof(ItermID));
            ofile.write(buffer, length);
            delete[] buffer;

            //Update metadata
            *metai = shift_metadata(*metai, shift);
            metai->filename = dir + namebaseo;

            if(!ifilestream.read(reinterpret_cast<char *>(&ItermID), sizeof(ItermID))) break;
        }
        else if(ZtermID < ItermID) {
            if( positional ) metaz = exlex.getPositional(ZtermID, dir+namebase1);
            else metaz = exlex.getNonPositional(ZtermID, dir+namebase1);
            //Calculate shift to use for updating the metadata
            long shift = ofile.tellp() - metaz->start_pos;
            //Subtract four since we already read the termID
            int length = metaz->end_offset - metaz->start_pos - sizeof(unsigned int);
            char* buffer = new char [length];
            zfilestream.read(buffer, length);
            //Write termID then rest of posting list
            ofile.write(reinterpret_cast<const char *>(&ZtermID), sizeof(ZtermID));
            ofile.write(buffer, length);
            delete[] buffer;

            //Update metadata
            *metaz = shift_metadata(*metaz, shift);
            metaz->filename = dir + namebaseo;

            if(!zfilestream.read(reinterpret_cast<char *>(&ZtermID), sizeof(ZtermID))) break;
        }
        else {
            if(positional) {
                metaz = exlex.getPositional(ZtermID, dir+namebase1);
                metai = exlex.getPositional(ItermID, dir+namebase2);
            }
            else {
                metaz = exlex.getNonPositional(ZtermID, dir+namebase1);
                metai = exlex.getNonPositional(ItermID, dir+namebase2);
            }

            if(positional) {
                //read both posting lists from both files
                std::vector<Posting> zpostinglist = read_pos_postinglist(zfilestream, metaz, ZtermID);
                std::vector<Posting> ipostinglist = read_pos_postinglist(ifilestream, metai, ItermID);

                //merge the posting lists
                std::vector<Posting> merged = merge_pos_postinglist(zpostinglist, ipostinglist);

                //write the final posting list to disk, creating a new metadata entry
                std::string outputfilepath = dir+namebaseo;
                write_postinglist<Posting>(ofile, outputfilepath, ZtermID, merged, true);

                //delete old metadata from both files
                exlex.deletePositional(ZtermID, exlex.getPositional(ZtermID, dir+namebase1));
                exlex.deletePositional(ItermID, exlex.getPositional(ItermID, dir+namebase2));
            }
            else {
                //read both posting lists from both files
                std::vector<nPosting> zpostinglist = read_nonpos_postinglist(zfilestream, metaz, ZtermID);
                std::vector<nPosting> ipostinglist = read_nonpos_postinglist(ifilestream, metai, ItermID);
                
                //merge the posting lists
                std::vector<nPosting> merged = merge_nonpos_postinglist(zpostinglist, ipostinglist);

                //write the final posting list to disk, creating a new metadata entry
                std::string outputfilepath = dir+namebaseo;
                write_postinglist<nPosting>(ofile, outputfilepath, ZtermID, merged, false);

                //delete old metadata from both files
                exlex.deleteNonPositional(ZtermID, exlex.getNonPositional(ZtermID, dir+namebase1));
                exlex.deleteNonPositional(ItermID, exlex.getNonPositional(ItermID, dir+namebase2));
            }
        }
    }
    if(zfilestream) {
        if( positional ) metaz = exlex.getPositional(ZtermID, dir+namebase1);
        else metaz = exlex.getNonPositional(ZtermID, dir+namebase1);
        //Calculate shift to use for updating the metadata
        long shift = ofile.tellp() - metaz->start_pos;
        //Subtract four since we already read the termID
        int length = metaz->end_offset - metaz->start_pos - sizeof(unsigned int);
        char* buffer = new char [length];
        zfilestream.read(buffer, length);
        //Write termID then rest of posting list
        ofile.write(reinterpret_cast<const char *>(&ZtermID), sizeof(ZtermID));
        ofile.write(buffer, length);
        delete[] buffer;

        //Update metadata
        *metaz = shift_metadata(*metaz, shift);
        metaz->filename = dir + namebaseo;
    }
    if(ifilestream) {
        if( positional ) metai = exlex.getPositional(ItermID, dir+namebase2);
        else metai = exlex.getNonPositional(ItermID, dir+namebase2);
        //Calculate shift to use for updating the metadata
        long shift = ofile.tellp() - metai->start_pos;
        //Subtract four since we already read the termID
        int length = metai->end_offset - metai->start_pos - sizeof(unsigned int);
        char* buffer = new char [length];
        ifilestream.read(buffer, length);
        //Write termID then rest of posting list
        ofile.write(reinterpret_cast<const char *>(&ItermID), sizeof(ItermID));
        ofile.write(buffer, length);
        delete[] buffer;

        //Update metadata
        *metai = shift_metadata(*metai, shift);
        metai->filename = dir + namebaseo;
    }

    zfilestream.close();
    ifilestream.close();
    ofile.close();
    std::string filename1 = dir + "Z" + std::to_string(indexnum);
    std::string filename2 = dir + "I" + std::to_string(indexnum);
    //deleting two files
    if( remove( filename1.c_str() ) != 0 ) std::cout << "Error deleting file" << std::endl;
    if( remove( filename2.c_str() ) != 0 ) std::cout << "Error deleting file" << std::endl;
}

std::vector<Posting> StaticIndex::merge_pos_postinglist(std::vector<Posting>& listz, std::vector<Posting>& listi) {
    std::vector<Posting> finallist(listz.size() + listi.size());
    auto ziter = listz.begin();
    auto iiter = listi.begin();

    while(ziter != listz.end() && iiter != listi.end()) {
        if(ziter->docID < iiter->docID) {
            finallist.push_back(*ziter);
            ziter++;
        }
        else if(iiter->docID < ziter->docID) {
            finallist.push_back(*iiter);
            iiter++;
        }
        else {
            //TODO: Determine how postings should be cleaned here
            finallist.push_back(*ziter);
            finallist.push_back(*iiter);
            ziter++;
            iiter++;
        }
    }
    while(ziter != listz.end()) {
        finallist.push_back(*ziter);
        ziter++;
    }
    while(iiter != listi.end()) {
        finallist.push_back(*iiter);
        iiter++;
    }
    return finallist;
}

std::vector<nPosting> StaticIndex::merge_nonpos_postinglist(std::vector<nPosting>& listz, std::vector<nPosting>& listi) {
    std::vector<nPosting> finallist(listz.size() + listi.size());
    auto ziter = listz.begin();
    auto iiter = listi.begin();

    while(ziter != listz.end() && iiter != listi.end()) {
        if(ziter->docID < iiter->docID) {
            finallist.push_back(*ziter);
            ziter++;
        }
        else if(iiter->docID < ziter->docID) {
            finallist.push_back(*iiter);
            iiter++;
        }
        else {
            //TODO: Determine how postings should be cleaned here
            nPosting tempposting = *ziter;
            tempposting.second += iiter->second;
            finallist.push_back(tempposting);
            ziter++;
            iiter++;
        }
    }
    while(ziter != listz.end()) {
        finallist.push_back(*ziter);
        ziter++;
    }
    while(iiter != listi.end()) {
        finallist.push_back(*iiter);
        iiter++;
    }
    return finallist;
}