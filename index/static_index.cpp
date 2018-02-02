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
unsigned int StaticIndex::write_block(std::vector<T> num, std::ofstream& ofile){
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
                std::cerr << "negative during delta compressiong\n";
            deltaencode.push_back(field[i] - field[i-1]);
        }
        compressed = encoder(deltaencode);
    }
    else {
        compressed = encoder(field);
    }
    return compressed;
}

//Writes an inverted index to disk using compressed postings
//filepath: The filename of the file being written to
//          INCLUDES THE PATH
template <typename T>
void StaticIndex::write_index(std::string filepath, std::ofstream& ofile, bool positional, T indexbegin, T indexend) {
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
        ofile.write(reinterpret_cast<const char *>(&termID), sizeof(termID));
        ofile.write(reinterpret_cast<const char *>(&(postinglistiter->second.size())), sizeof(postinglistiter->second.size()));
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

        while(blockend <= postinglistiter->second.size()) {
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
            if(positional) std::vector<uint8_t> compressedthird = compress_block(blockthird, VBEncode, false);

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
        if(blockbegin != postinglistiter->second.size()) {
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
            if(positional) std::vector<uint8_t> compressedthird = compress_block(blockthird, VBEncode, false);

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
        write_block<unsigned int>(lastdocID, ofile);
        metadata.blocksizes = ofile.tellp();
        write_block<unsigned int>(blocksizes, ofile);
        metadata.postings_blocks = ofile.tellp();

        for(auto iter = compressedblocks.begin(); iter != compressedblocks.end(); iter++) {
            write_block<uint8_t>(*iter, ofile);
        }

        metadata.end_offset = ofile.tellp();
        if(positional) exlex.addPositional(termID, metadata);
        else exlex.addNonPositional(termID, metadata);
    }
}

StaticIndex::Pos_Index StaticIndex::read_positional_index(std::ifstream& ifile, std::string filename) {
    Pos_Index index;
    std::string filepath = std::string(posdir) + filename;

    ifile.seekg(0,std::ios::beg);

    unsigned int readtermID;

    while(ifile.read(reinterpret_cast<char *>(&readtermID), sizeof(readtermID))) {
        auto meta = exlex.getPositional(readtermID, filepath);

        unsigned int doc_method, second_method, third_method, postinglistlength;

        ifile.read(reinterpret_cast<char *>(&postinglistlength), sizeof(postinglistlength));
        ifile.read(reinterpret_cast<char *>(&doc_method), sizeof(doc_method));
        ifile.read(reinterpret_cast<char *>(&second_method), sizeof(second_method));
        ifile.read(reinterpret_cast<char *>(&third_method), sizeof(third_method));

        //Read the blocksize array
        ifile.seekg(meta->blocksizes);
        unsigned int buffersize = meta->end_offset - meta->blocksizes;
        std::vector<char> buffer(buffersize);
        ifile.read(&buffer[0], buffersize);
        std::vector<unsigned int> blocksizes = VBDecode(std::vector<uint8_t>(buffer.begin(), buffer.end()));
        buffer.clear();

        if(blocksizes.size() % 3 != 0) {
            throw std::invalid_argument("Error, blocksize array is not a multiple of 3: " + std::to_string(blocksizes.size()));
        }

        ifile.seekg(meta->postings_blocks);
        //For every three blocksize entries, read in three blocks of numbers and insert postings into the index
        for(size_t i = 0; i < blocksizes.size(); i += 3) {
            unsigned int doclength = blocksizes[i];
            unsigned int secondlength = blocksizes[i+1];
            unsigned int thirdlength = blocksizes[i+2];
        
            std::vector<unsigned int> docIDs, secondvec, thirdvec;

            buffer.resize(doclength);
            ifile.read(&buffer[0], doclength);
            docIDs = VBDecode(std::vector<uint8_t>(buffer.begin(), buffer.end()));
            buffer.clear();

            buffer.resize(secondlength);
            ifile.read(&buffer[0], secondlength);
            secondvec = VBDecode(std::vector<uint8_t>(buffer.begin(), buffer.end()));
            buffer.clear();

            buffer.resize(thirdlength);
            ifile.read(&buffer[0], thirdlength);
            thirdvec = VBDecode(std::vector<uint8_t>(buffer.begin(), buffer.end()));
            buffer.clear();

            if(docIDs.size() != secondvec.size() || secondvec.size() != thirdvec.size()) {
                throw std::invalid_argument("Error, vectors mismatched in size while reading index: " + std::to_string(docIDs.size()) + "," + std::to_string(secondvec.size()) + "," + std::to_string(thirdvec.size()));
            }

            for(size_t j = 0; j < docIDs.size(); j++) {
                Posting newpost;
                newpost.termID = readtermID;
                newpost.docID = docIDs[j];
                newpost.second = secondvec[j];
                newpost.third = thirdvec[j];

                index[readtermID].push_back(newpost);
            }
        }

        if(index[readtermID].size() != postinglistlength) {
            throw std::invalid_argument("Error, posting list length not equal to specified length: " + std::to_string(index[readtermID].size()) + "," + std::to_string(postinglistlength));
        }
    }

    return index;
}

StaticIndex::NonPos_Index StaticIndex::read_nonpositional_index(std::ifstream& ifile, std::string filename) {
    NonPos_Index index;
    std::string filepath = std::string(nonposdir) + filename;

    ifile.seekg(0,std::ios::beg);

    unsigned int readtermID;

    while(ifile.read(reinterpret_cast<char *>(&readtermID), sizeof(readtermID))) {
        auto meta = exlex.getNonPositional(readtermID, filepath);

        unsigned int doc_method, second_method, postinglistlength;

        ifile.read(reinterpret_cast<char *>(&postinglistlength), sizeof(postinglistlength));
        ifile.read(reinterpret_cast<char *>(&doc_method), sizeof(doc_method));
        ifile.read(reinterpret_cast<char *>(&second_method), sizeof(second_method));

        //Read the blocksize array
        ifile.seekg(meta->blocksizes);
        unsigned int buffersize = meta->end_offset - meta->blocksizes;
        std::vector<char> buffer(buffersize);
        ifile.read(&buffer[0], buffersize);
        std::vector<unsigned int> blocksizes = VBDecode(std::vector<uint8_t>(buffer.begin(), buffer.end()));
        buffer.clear();

        if(blocksizes.size() % 2 != 0) {
            throw std::invalid_argument("Error, blocksize array is not a multiple of 2: " + std::to_string(blocksizes.size()));
        }

        ifile.seekg(meta->postings_blocks);
        //For every three blocksize entries, read in three blocks of numbers and insert postings into the index
        for(size_t i = 0; i < blocksizes.size(); i += 2) {
            unsigned int doclength = blocksizes[i];
            unsigned int secondlength = blocksizes[i+1];
        
            std::vector<unsigned int> docIDs, secondvec;

            buffer.resize(doclength);
            ifile.read(&buffer[0], doclength);
            docIDs = VBDecode(std::vector<uint8_t>(buffer.begin(), buffer.end()));
            buffer.clear();

            buffer.resize(secondlength);
            ifile.read(&buffer[0], secondlength);
            secondvec = VBDecode(std::vector<uint8_t>(buffer.begin(), buffer.end()));
            buffer.clear();

            if(docIDs.size() != secondvec.size()) {
                throw std::invalid_argument("Error, vectors mismatched in size while reading index: " + std::to_string(docIDs.size()) + "," + std::to_string(secondvec.size()));
            }

            for(size_t j = 0; j < docIDs.size(); j++) {
                nPosting newpost;
                newpost.termID = readtermID;
                newpost.docID = docIDs[j];
                newpost.second = secondvec[j];

                index[readtermID].push_back(newpost);
            }
        }

        if(index[readtermID].size() != postinglistlength) {
            throw std::invalid_argument("Error, posting list length not equal to specified length: " + std::to_string(index[readtermID].size()) + "," + std::to_string(postinglistlength));
        }
    }

    return index;
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

void StaticIndex::newmerge(int indexnum, bool positional) {
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

    std::vector<mData>::iterator metai, metaz;
    unsigned int termIDZ, termIDI;
    }
}

/**
 * Merges the indexes of the given order. Both the Z-index and I-index must already exist before
 * this method is called.
 */
void StaticIndex::merge(int indexnum, int positional){

    std::ifstream filez;
    std::ifstream filei;
    std::ofstream ofile;
    std::string dir;
    if(positional) dir = posdir;
    else dir = nonposdir;

    //determine the name of the output file, if "Z" file exists, than compressed to "I" file.
    char flag = 'Z';
    filez.open(dir + "Z" + std::to_string(indexnum));
    filei.open(dir + "I" + std::to_string(indexnum));

    std::string namebase1 = "Z" + std::to_string(indexnum);
    std::string namebase2 = "I" + std::to_string(indexnum);
    std::string namebaseo;

    std::ifstream filetest(dir + "Z" + std::to_string(indexnum+1));
    if(filetest.good())
        flag = 'I';
    filetest.close();

    namebaseo = flag + std::to_string(indexnum + 1);
    ofile.open(dir + namebaseo);

    if(positional)
        std::cout << "Positional is ";
    else
        std::cout << "Non-Positional is ";
    std::cout << "merging into " << flag << indexnum + 1 << "------------------------------------" << std::endl;

    /* To merge, first read the first integer from file, which is the termID.
        Compare the termID, copy and paste the postings of smaller termID to the output file.
        This is basically a next-greater-or-equal-to process.
        If both file contains same termID, need to decompress and merge posting.
        Decompress method push all the posting to the dynamic index.
    */
    std::vector<mData>::iterator metai, metaz;
    unsigned int termIDZ, termIDI;
    filez.read(reinterpret_cast<char *>(&termIDZ), sizeof(termIDZ));
    filei.read(reinterpret_cast<char *>(&termIDI), sizeof(termIDI));
    if( filez.is_open() && filei.is_open() ){
        while(  !filez.eof() && !filei.eof() ){
            std::cout << "TermIDZ " << termIDZ << " TermIDI "<< termIDI << std::endl;
            if( termIDZ < termIDI ){
                if( positional ) metaz = exlex.getPositional(termIDZ, dir+namebase1);
                else metaz = exlex.getNonPositional(termIDZ, dir+namebase1);
                //Calculate shift to use for updating the metadata
                long shift = ofile.tellp() - metaz->start_pos;
                //Subtract four since we already read the termID
                int length = metaz->end_offset - metaz->start_pos - sizeof(unsigned int);
                char* buffer = new char [length];
                filez.read(buffer, length);
                ofile.write(buffer, length);
                delete[] buffer;

                //Update metadata
                *metaz = shift_metadata(*metaz, shift);
                metaz->filename = dir + namebaseo;

                filez.read(reinterpret_cast<char *>(&termIDZ), sizeof(termIDZ));
            }
            else if( termIDI < termIDZ ){
                if( positional ) metai = exlex.getPositional(termIDI, dir+namebase2);
                else metai = exlex.getNonPositional(termIDI, dir+namebase2);
                //Calculate shift to use for updating the metadata
                long shift = ofile.tellp() - metai->start_pos;
                //Subtract four since we already read the termID
                int length = metai->end_offset - metai->start_pos - sizeof(unsigned int);
                char* buffer = new char [length];
                filei.read(buffer, length);
                ofile.write(buffer, length);
                delete[] buffer;

                //Update metadata
                *metai = shift_metadata(*metai, shift);
                metai->filename = dir + namebaseo;

                filei.read(reinterpret_cast<char *>(&termIDI), sizeof(termIDI));
            }
            else if( termIDI == termIDZ ){
                if( positional ){
                    auto indexZ = decompress_p_posting(termIDZ, filez, namebase1);
                    auto indexI = decompress_p_posting(termIDI, filei, namebase2);
                    auto positional_index = merge_positional_index(indexZ, indexI);

                    Pos_Map_Iter ite = positional_index.begin();
                    Pos_Map_Iter end = positional_index.end();
                    auto vit = ite->second.begin();
                    auto vend = ite->second.end();
                    write_compressed_index<Pos_Map_Iter, std::vector<Posting>::iterator>(dir + namebaseo, ofile, ite, end, vit, vend, 1);
                    //Delete the metadata associated with the two termIDs for each file
                    exlex.deletePositional(termIDZ, exlex.getPositional(termIDZ, dir+namebase1));
                    exlex.deletePositional(termIDI, exlex.getPositional(termIDI, dir+namebase2));
                }
                else{
                    auto nonpositional_index = decompress_np_posting(termIDI, filez, filei, namebase1, namebase2);
                    NonPos_Map_Iter ite = nonpositional_index.begin();
                    NonPos_Map_Iter end = nonpositional_index.end();
                    auto vit = ite->second.begin();
                    auto vend = ite->second.end();
                    write_compressed_index<NonPos_Map_Iter, std::vector<nPosting>::iterator>(dir + namebaseo, ofile, ite, end, vit, vend, 0);
                    //Delete the metadata associated with the two termIDs for each file
                    exlex.deleteNonPositional(termIDZ, exlex.getNonPositional(termIDZ, dir+namebase1));
                    exlex.deleteNonPositional(termIDI, exlex.getNonPositional(termIDI, dir+namebase2));
                }
                filez.read(reinterpret_cast<char *>(&termIDZ), sizeof(termIDZ));
                filei.read(reinterpret_cast<char *>(&termIDI), sizeof(termIDI));
            }
        }
    }
    else std::cerr << "Error opening file." << std::endl;

    filez.close();
    filei.close();
    ofile.close();
    std::string filename1 = dir + "Z" + std::to_string(indexnum);
    std::string filename2 = dir + "I" + std::to_string(indexnum);
    //deleting two files
    if( remove( filename1.c_str() ) != 0 ) std::cout << "Error deleting file" << std::endl;
    if( remove( filename2.c_str() ) != 0 ) std::cout << "Error deleting file" << std::endl;
}

StaticIndex::Pos_Index StaticIndex::merge_positional_index(StaticIndex::Pos_Index& indexZ, StaticIndex::Pos_Index& indexI) {
    Pos_Index mergedindex;
    auto Ziter = indexZ.begin();
    auto Iiter = indexI.begin();

    while(Ziter != indexZ.end() && Iiter != indexI.end()) {
        if(Ziter->first > Iiter->first) {
            mergedindex[Iiter->first] = Iiter->second;
            ++Iiter;
        }
        else if(Ziter->first < Iiter->first) {
            mergedindex[Ziter->first] = Ziter->second;
            ++Ziter;
        }
        //Ziter == Iiter
        else {
            std::vector<Posting> newlist;

            auto Zveciter = Ziter->second.begin();
            auto Iveciter = Iiter->second.begin();
            while(Zveciter != Ziter->second.end() && Iveciter != Iiter->second.end()) {
                if(Zveciter->docID > Iveciter->docID) {
                    newlist.push_back(*Iveciter);
                    ++Iveciter;
                }
                else if(Zveciter->docID < Iveciter->docID) {
                    newlist.push_back(*Zveciter);
                    ++Zveciter;
                }
                //TODO: Determine how postings should be cleaned here
                else {
                    newlist.push_back(*Zveciter);
                    newlist.push_back(*Iveciter);
                    ++Zveciter;
                    ++Iveciter;
                }
            }

            //Either the Z-vector is empty, or the I-vector is empty. In either case we empty out the non-empty vector
            while(Zveciter != Ziter->second.end()) {
                newlist.push_back(*Zveciter);
                ++Zveciter;
            }
            while(Iveciter != Iiter->second.end()) {
                newlist.push_back(*Iveciter);
                ++Iveciter;
            }

            mergedindex.emplace(Ziter->first, std::move(newlist));

            ++Iiter;
            ++Ziter;
        }
    }

    return mergedindex;
}
