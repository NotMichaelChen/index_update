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
        write_compressed_index<NonPos_Map_Iter>(filename, ofile, false, indexbegin, indexend);

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

    for(auto postinglistiter = indexbegin; postinglistiter != indexend; postinglistiter++) {
        mData metadata;
        metadata.filename = filepath;
        metadata.start_pos = ofile.tellp();

        unsigned int termID = postinglistiter->first;

        //Write out metadata
        ofile.write(reinterpret_cast<const char *>(&termID), sizeof(termID));
        ofile.write(reinterpret_cast<const char *>(&(postinglistiter->second.size())), sizeof(postinglistiter->second.size()));
        ofile.write(reinterpret_cast<const char *>(&doc_method), sizeof(doc_method));
        ofile.write(reinterpret_cast<const char *>(&second_method), sizeof(second_method));
        if(positional) ofile.write(reinterpret_cast<const char *>(&third_method), sizeof(third_method));

        //Construct arrays holding the last entry in each docID block and the size of all blocks

        //Last entry of each docID block will happen every blocksize number of postings (every 128th posting for example)
        std::vector<unsigned int> lastdocID;
        std::vector<unsigned int> blocksizes;
        
        size_t arraypos = blocksize-1;
        if(arraypos > postinglistiter->second.size()) {
            lastdocID.push_back(postinglistiter->second[postinglistiter->second.size()-1].docID);
            blocksizes.push_back(postinglistiter->second.size());
        }

        //No need for else; while will be skipped if if is true
        while(arraypos < postinglistiter->second.size()) {
            if(arraypos+blocksize < postinglistiter->second.size()) {
                arraypos += blocksize;
                lastdocID.push_back(postinglistiter->second[arraypos].docID);
                blocksizes.push_back(blocksize);
            }
            else {
                if(arraypos != postinglistiter->second.size()-1) {
                    lastdocID.push_back(postinglistiter->second[postinglistiter->second.size()-1].docID);
                    blocksizes.push_back((postinglistiter->second.size()-1) - arraypos);
                }
                break;
            }
        }

        metadata.last_docID = ofile.tellp();
        write_block<unsigned int>(lastdocID, ofile);
        metadata.blocksizes = ofile.tellp();
        write_block<unsigned int>(blocksizes, ofile);
        metadata.postings_blocks = ofile.tellp();

        //Write out alternating blocks

        //Begin inclusive, End exclusive
        size_t blockbegin = 0;
        size_t blockend = blocksize;

        while(blockend <= postinglistiter->second.size()) {
            std::vector<unsigned int> blockdocID;
            std::vector<unsigned int> blocksecond;
            std::vector<unsigned int> blockthird;

            for(auto postingiter = postinglistiter->second.begin() + blockbegin; postingiter != postinglistiter->second.begin() + blockend; postingiter++) {
                blockdocID.push_back(postingiter->docID);
                blocksecond.push_back(postingiter->second);
                if(positional) blockthird.push_back(postingiter->third);
            }

            std::vector<uint8_t> compresseddocID = compress_block(blockdocID, VBEncode, true);
            std::vector<uint8_t> compressedsecond = compress_block(blocksecond, VBEncode, false);
            if(positional) std::vector<uint8_t> compressedthird = compress_block(blockthird, VBEncode, false);

            write_block<uint8_t>(compresseddocID, ofile);
            write_block<uint8_t>(compressedsecond, ofile);
            if(positional) write_block<uint8_t>(compressedthird, ofile);

            blockbegin += blocksize;
            blockend += blocksize;
        }

        if(blockbegin != postinglistiter->second.size()) {
            std::vector<unsigned int> blockdocID;
            std::vector<unsigned int> blocksecond;
            std::vector<unsigned int> blockthird;

            for(auto postingiter = postinglistiter->second.begin() + blockbegin; postingiter != postinglistiter->second.end(); postingiter++) {
                blockdocID.push_back(postingiter->docID);
                blocksecond.push_back(postingiter->second);
                if(positional) blockthird.push_back(postingiter->third);
            }

            std::vector<uint8_t> compresseddocID = compress_block(blockdocID, VBEncode, true);
            std::vector<uint8_t> compressedsecond = compress_block(blocksecond, VBEncode, false);
            std::vector<uint8_t> compressedthird = compress_block(blockthird, VBEncode, false);

            write_block<uint8_t>(compresseddocID, ofile);
            write_block<uint8_t>(compressedsecond, ofile);
            write_block<uint8_t>(compressedthird, ofile);
        }

        metadata.end_offset = ofile.tellp();
        if(positional) exlex.addPositional(termID, metadata);
        else exlex.addNonPositional(termID, metadata);
    }
}

StaticIndex::Pos_Index StaticIndex::decompress_p_posting(unsigned int termID, std::ifstream& ifile, std::string namebase){
    /* Decompress positional postings and store them in map structure
        Since the last block may not necessarily contain 128 elements; need to find how many elements
        in the last block before adding them to respective vector. */
    std::string filename = std::string(posdir) + namebase;
    auto meta = exlex.getPositional(termID, filename);

    int method1, method2, method3;

    ifile.read(reinterpret_cast<char *>(&method1), sizeof(method1));
    ifile.read(reinterpret_cast<char *>(&method2), sizeof(method2));
    ifile.read(reinterpret_cast<char *>(&method3), sizeof(method3));

    ifile.seekg(meta->posting_offset);
    std::vector<unsigned int> docID, fragID, pos;
    int count;
    //read all the alternating blocks from compressed index and decompress
    int length = meta->docID_end - meta->posting_offset; //full blocks plus last docID block
    char* buffer = new char [length];
    ifile.read(buffer, length);
    std::vector<unsigned int> decompressed = VBDecode(buffer, length);
    delete[] buffer;
    int last_block = decompressed.size() % blocksize;//find the how many elements in the last block
    std::vector<unsigned int>::iterator it = decompressed.begin();
    unsigned int prevID;
    //TODO: here assume only doc ID is delta encoded; the other two can also be delta encoded,
    while( it != decompressed.end() ){
        count = 0;
        prevID = 0;
        while( count < blocksize && it != decompressed.end()){
            docID.push_back( prevID + *it );
            prevID = *it;
            count ++;
            it ++;
        }
        count = 0;
        while( count < blocksize && it != decompressed.end()){
            fragID.push_back(*it);
            count ++;
            it ++;
        }
        count = 0;
        while( count < blocksize && it != decompressed.end()){
            pos.push_back(*it);
            count ++;
            it ++;
        }
    }
    //add elements from last two blocks
    decompressed.clear();
    length = meta->postingCount_offset - meta->docID_end;
    buffer = new char [length];
    ifile.read(buffer, length);
    decompressed = VBDecode(buffer, length);
    delete[] buffer;
    it = decompressed.begin();
    count = 0;
    fragID.insert(fragID.end(), it, it + last_block);
    pos.insert(pos.end(), it + last_block, decompressed.end());

    int docsize = docID.size();
    int fragsize = fragID.size();
    int possize = pos.size();
    if( docsize != fragsize || fragsize != possize ) 
        std::cerr << "Error: number of postings doesn't match: " << docsize << ' ' << fragsize << ' ' << possize << std::endl;

    std::vector<unsigned int>::iterator it1 = docID.begin();
    std::vector<unsigned int>::iterator it2 = fragID.begin();
    std::vector<unsigned int>::iterator it3 = pos.begin();
    Posting p;
    Pos_Index positional_index;
    while( it1 != docID.end() ){
        p.termID = termID;
        p.docID = *it1;
        p.second = *it2;
        p.third = *it3;
        positional_index[termID].push_back(p);
        it1 ++;
        it2 ++;
        it3 ++;
    }

    ifile.seekg(meta->end_offset);

    return positional_index;
}

StaticIndex::NonPos_Index StaticIndex::decompress_np_posting(unsigned int termID, std::ifstream& filez,
    std::ifstream& filei, std::string namebase1, std::string namebase2) {
    int doc_methodi, second_methodi, doc_methodz, second_methodz;

    //assume termID already read
    filez.read(reinterpret_cast<char *>(&doc_methodz), sizeof(doc_methodz));
    filez.read(reinterpret_cast<char *>(&second_methodz), sizeof(second_methodz));
    filei.read(reinterpret_cast<char *>(&doc_methodi), sizeof(doc_methodi));
    filei.read(reinterpret_cast<char *>(&second_methodi), sizeof(second_methodi));

    auto metaz = exlex.getNonPositional(termID, nonposdir+namebase1);
    auto metai = exlex.getNonPositional(termID, nonposdir+namebase2);

    filez.seekg(metaz->posting_offset);
    filei.seekg(metai->posting_offset);
    std::vector<unsigned int> docIDi, docIDz, freqi, freqz;
    int count;
    //read all the alternating blocks from compressed index and decompress
    int lengthz = metaz->docID_end - metaz->posting_offset;
    int lengthi = metai->docID_end - metai->posting_offset;
    char* bufferz = new char [lengthz];
    char* bufferi = new char [lengthi];
    filez.read(bufferz, lengthz);
    filei.read(bufferi, lengthi);
    std::vector<unsigned int> decomz = VBDecode(bufferz, lengthz);
    std::vector<unsigned int> decomi = VBDecode(bufferi, lengthi);
    delete[] bufferi;
    delete[] bufferz;
    int last_block_z = decomz.size() % 128;
    int last_block_i = decomi.size() % 128;
    std::vector<unsigned int>::iterator itz = decomz.begin();
    std::vector<unsigned int>::iterator iti = decomi.begin();
    unsigned int prevIDz, prevIDi;

    while( itz != decomz.end() ){
        count = 0;
        prevIDz = 0;
        while( count < blocksize && itz != decomz.end() ){
            prevIDz = prevIDz + *itz;
            docIDz.push_back( prevIDz );
            count ++;
            itz ++;
        }
        count = 0;
        while( count < blocksize && itz != decomz.end() ){
            freqz.push_back(*itz);
            count ++;
            itz ++;
        }
    }
    while( iti != decomi.end() ){
        count = 0;
        prevIDi = 0;
        while( count < blocksize && iti != decomi.end() ){
            prevIDi = prevIDi + *iti;
            docIDi.push_back( prevIDi );
            count ++;
            iti ++;
        }
        count = 0;
        while( count < blocksize && iti != decomi.end() ){
            freqi.push_back(*iti);
            count ++;
            iti ++;
        }
    }
    //add the last block
    decomi.clear();
    decomz.clear();
    lengthz = metaz->postingCount_offset - metaz->docID_end;
    lengthi = metai->postingCount_offset - metai->docID_end;
    bufferz = new char [lengthz];
    bufferi = new char [lengthi];
    filez.read(bufferz, lengthz);
    filei.read(bufferi, lengthi);
    decomz = VBDecode(bufferz, lengthz);
    decomi = VBDecode(bufferi, lengthi);
    freqi.insert(freqi.end(), decomi.begin(), decomi.end());
    freqz.insert(freqz.end(), decomz.begin(), decomz.end());
    delete[] bufferi;
    delete[] bufferz;

    itz = docIDz.begin();
    iti = docIDi.begin();
    std::vector<unsigned int>::iterator itfz = freqz.begin();
    std::vector<unsigned int>::iterator itfi = freqi.begin();
    nPosting p;
    NonPos_Index nonpositional_index;
    while( itz != docIDz.end() && iti != docIDi.end() ){
        if( *itz > *iti ){
            p.docID = *itz;
            p.second = *itfz;
            itz ++;
            itfz ++;
            nonpositional_index[termID].push_back(p);
        }
        else if( *itz < *iti ){
            p.docID = *iti;
            p.second = *itfi;
            iti ++;
            itfi ++;
            nonpositional_index[termID].push_back(p);
        }
        else{
            //if equal, use the frequency of I file
            p.docID = *iti;
            p.second = *itfi;
            iti ++;
            itz ++;
            itfi ++;
            itfz ++;
            nonpositional_index[termID].push_back(p);
        }

    }
    if( itz != docIDz.end() ){
        p.docID = *itz;
        p.second = *itfz;
        itz ++;
        itfz ++;
        nonpositional_index[termID].push_back(p);
    }
    else if( iti != docIDi.end() ){
        p.docID = *iti;
        p.second = *itfi;
        iti ++;
        itfi ++;
        nonpositional_index[termID].push_back(p);
    }

    filez.seekg(metaz->end_offset);
    filei.seekg(metai->end_offset);
    return nonpositional_index;
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
