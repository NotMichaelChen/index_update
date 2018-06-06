#include "static_index.hpp"

#include <iostream>
#include <functional>
#include <algorithm>

#include "static_functions/postingIO.hpp"
#include "util.hpp"

//Copies n bytes from the ifstream to the ofstream
int copyBytes(std::ifstream& ifile, std::ofstream& ofile, int n) {
    char* buffer = new char[n];
    ifile.read(buffer, n);
    ofile.write(buffer, ifile.gcount());
    delete[] buffer;
    if(ifile.gcount() != n) {
        return 1;
    }
    return 0;
}

StaticIndex::StaticIndex(std::string& working_dir) : INDEXDIR("./" + working_dir + GlobalConst::IndexPath),
    PDIR("./" + working_dir + GlobalConst::PosPath),
    NPDIR("./" + working_dir + GlobalConst::NonPosPath)
{}

ExtendedLexicon* StaticIndex::getExlexPointer() {
    return &exlex;
}

//Writes the positional index to disk, which means it is saved either in file Z0 or I0.
void StaticIndex::write_p_disk(GlobalType::PosMapIter indexbegin, GlobalType::PosMapIter indexend) {
    std::string filename = PDIR;
    std::string indexname;
    //Z0 exists
    if(std::ifstream(filename + "Z0"))
        indexname = "I0";
    else
        indexname = "Z0";

    std::ofstream ofile(filename + indexname);

    if (ofile.is_open()){
        write_index<GlobalType::PosMapIter>(indexname, ofile, true, indexbegin, indexend);

        ofile.close();
    }else{
        std::cerr << "File cannot be opened." << std::endl;
    }

    merge_test(true);
}

//Writes the non-positional index to disk, which is saved in either file Z0 or I0
void StaticIndex::write_np_disk(GlobalType::NonPosMapIter indexbegin, GlobalType::NonPosMapIter indexend) {
    std::string filename = NPDIR;
    std::string indexname;
    //Z0 exists
    if(std::ifstream(filename + "Z0"))
        indexname = "I0";
    else
        indexname = "Z0";

    std::ofstream ofile(filename + indexname);

    if (ofile.is_open()){
        write_index<GlobalType::NonPosMapIter>(indexname, ofile, false, indexbegin, indexend);

        ofile.close();
    }else{
        std::cerr << "File cannot be opened." << std::endl;
    }

    merge_test(false);
}

//Writes an inverted index to disk using compressed postings
//indexname: The name of the index being written to. Always follows the format (Z|I)(number)
template <typename T>
void StaticIndex::write_index(std::string& indexname, std::ofstream& ofile, bool positional, T indexbegin, T indexend) {
    if(indexname.empty())
        throw std::invalid_argument("Error, index name is empty");
    
    bool isZindex;
    if(indexname[0] = 'Z')
        isZindex = true;
    else if(indexname[0] = 'I')
        isZindex = false;
    else
        throw std::invalid_argument("Error, index name " + indexname + " is invalid");

    unsigned int indexnum = std::stoul(indexname.substr(1));

    //Counts how many postings have accumulated since the last pointer inserted in the extended lexicon
    size_t postingcount = 0;
    //Indicates whether the last list had a pointer due to size
    bool lastlisthadpointer = false;

    //for each posting list in the index
    for(auto postinglistiter = indexbegin; postinglistiter != indexend; postinglistiter++) {
        //Posting list is large enough to get an entry in the sparse lex
        if(postinglistiter->second.size() > SPARSE_SIZE) {
            if(positional)
                spexlex.insertPosEntry(postinglistiter->first, indexnum, isZindex, ofile.tellp());
            else
                spexlex.insertNonPosEntry(postinglistiter->first, indexnum, isZindex, ofile.tellp());

            postingcount = 0;
            lastlisthadpointer = true;
        }
        //Last posting list had an entry in the sparse lex
        else if(lastlisthadpointer) {
            if(positional)
                spexlex.insertPosEntry(postinglistiter->first, indexnum, isZindex, ofile.tellp());
            else
                spexlex.insertNonPosEntry(postinglistiter->first, indexnum, isZindex, ofile.tellp());
            
            postingcount += postinglistiter->second.size();
            lastlisthadpointer = false;
        }
        //Enough postings accumulated to insert a pointer
        else if(postingcount > SPARSE_BETWEEN_SIZE) {
            if(positional)
                spexlex.insertPosEntry(postinglistiter->first, indexnum, isZindex, ofile.tellp());
            else
                spexlex.insertNonPosEntry(postinglistiter->first, indexnum, isZindex, ofile.tellp());

            postingcount = 0;
        }
        else {
            postingcount += postinglistiter->second;
        }

        //Write out the posting list to disk
        write_postinglist(ofile, postinglistiter->first, postinglistiter->second, positional);
    }
}

/**
 * Test if there are two files of same index number on disk.
 * If there is, merge them and then call merge_test again until
 * all index numbers have only one file each.
 * Assumes that only one index is ever written to disk
 */
void StaticIndex::merge_test(bool isPositional) {
    //Assign directory the correct string based on the parameter
    std::string directory = isPositional ? PDIR : NPDIR;

    std::vector<std::string> files = Utility::readDirectory(directory);
    auto dir_iter = files.begin();

    while(dir_iter != files.end()) {
        //If any index file starts with an 'I', then we need to merge it
        if(dir_iter->size() > 1 && (*dir_iter)[0] == 'I') {
            //Get the number of the index
            int indexnum = std::stoi(dir_iter->substr(1));
            std::cerr << "Merging positional: " << isPositional << ", index number " << indexnum << std::endl;
            merge(indexnum, isPositional);

            files.clear();
            files = Utility::readDirectory(directory);
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
    std::string dir = positional ? PDIR : NPDIR;

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

    unsigned int ZtermID, ItermID;
    zfilestream.read(reinterpret_cast<char *>(&ZtermID), sizeof(ZtermID));
    ifilestream.read(reinterpret_cast<char *>(&ItermID), sizeof(ItermID));
    while(true) {
        if(ItermID < ZtermID) {
            //Read length of block
            unsigned int blocklen;
            ifilestream.read(reinterpret_cast<char *>(&blocklen), sizeof(blocklen));
            //Don't copy over the length and the termid
            blocklen -= 8;

            ofile.write(reinterpret_cast<const char *>(&ItermID), sizeof(ItermID));
            ofile.write(reinterpret_cast<const char *>(&blocklen), sizeof(blocklen));

            copyBytes(ifilestream, ofile, blocklen);

            if(!ifilestream.read(reinterpret_cast<char *>(&ItermID), sizeof(ItermID))) break;
        }
        else if(ZtermID < ItermID) {
            //Read length of block
            unsigned int blocklen;
            zfilestream.read(reinterpret_cast<char *>(&blocklen), sizeof(blocklen));
            //Don't copy over the length and the termid
            blocklen -= 8;

            ofile.write(reinterpret_cast<const char *>(&ZtermID), sizeof(ZtermID));
            ofile.write(reinterpret_cast<const char *>(&blocklen), sizeof(blocklen));

            copyBytes(zfilestream, ofile, blocklen);

            if(!zfilestream.read(reinterpret_cast<char *>(&ZtermID), sizeof(ZtermID))) break;
        }
        else {
            
            if(positional) {
                //read both posting lists from both files
                std::vector<Posting> zpostinglist = read_pos_postinglist(zfilestream, ZtermID);
                std::vector<Posting> ipostinglist = read_pos_postinglist(ifilestream, ItermID);

                //merge the posting lists
                std::vector<Posting> merged = merge_pos_postinglist(zpostinglist, ipostinglist);

                //write the final posting list to disk, creating a new metadata entry
                std::string outputfilepath = dir+namebaseo;
                write_postinglist<Posting>(ofile, ZtermID, merged, true);
            }
            else {
                //read both posting lists from both files
                std::vector<nPosting> zpostinglist = read_nonpos_postinglist(zfilestream, ZtermID);
                std::vector<nPosting> ipostinglist = read_nonpos_postinglist(ifilestream, ItermID);
                
                //merge the posting lists
                std::vector<nPosting> merged = merge_nonpos_postinglist(zpostinglist, ipostinglist);

                //write the final posting list to disk, creating a new metadata entry
                std::string outputfilepath = dir+namebaseo;
                write_postinglist<nPosting>(ofile, ZtermID, merged, false);
            }

            zfilestream.read(reinterpret_cast<char *>(&ZtermID), sizeof(ZtermID));
            ifilestream.read(reinterpret_cast<char *>(&ItermID), sizeof(ItermID));

            if(!zfilestream || !ifilestream) break;
        }
    }
    if(zfilestream) {
        do {
            //Read length of block
            unsigned int blocklen;
            zfilestream.read(reinterpret_cast<char *>(&blocklen), sizeof(blocklen));
            //Don't copy over the length and the termid
            blocklen -= 8;

            ofile.write(reinterpret_cast<const char *>(&ZtermID), sizeof(ZtermID));
            ofile.write(reinterpret_cast<const char *>(&blocklen), sizeof(blocklen));

            copyBytes(zfilestream, ofile, blocklen);
        } while (zfilestream.read(reinterpret_cast<char *>(&ZtermID), sizeof(ZtermID)));
    }
    if(ifilestream) {
        do {
            //Read length of block
            unsigned int blocklen;
            ifilestream.read(reinterpret_cast<char *>(&blocklen), sizeof(blocklen));
            //Don't copy over the length and the termid
            blocklen -= 8;

            ofile.write(reinterpret_cast<const char *>(&ItermID), sizeof(ItermID));
            ofile.write(reinterpret_cast<const char *>(&blocklen), sizeof(blocklen));

            copyBytes(ifilestream, ofile, blocklen);
        } while (ifilestream.read(reinterpret_cast<char *>(&ItermID), sizeof(ItermID)));
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
    std::vector<Posting> finallist;
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
    std::vector<nPosting> finallist;
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