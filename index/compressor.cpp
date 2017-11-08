/* Methods related to compressing posting list */
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <dirent.h>

#include "index.hpp"
#include "varbyte.hpp"

#define PDIR "./disk_index/positional/"//path to static positional index
#define NPDIR "./disk_index/non_positional/"//path to static non-positional index

std::vector<std::string> Index::read_directory( std::string path ){
    /**
     * List all the files in a directory.
     *
     * @param: the path of a directory
     * @return: the names of all the files in this directory.
     */
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

bool Index::check_contain(std::vector<std::string> v, std::string f){
	//check if a vector contains an element
    for( std::vector<std::string>::iterator it = v.begin(); it != v.end(); it ++){
        if( *it == f) return true;
    }
    return false;
}

template <typename T>
void Index::write(std::vector<T> num, std::ofstream& ofile){
	/**
	 * Write the compressed posting to file byte by byte.
	 */
	for(typename std::vector<T>::iterator it = num.begin(); it != num.end(); it++){
		ofile.write(reinterpret_cast<const char*>(&(*it)), sizeof(*it));
	}
}

template <typename T>
void Index::compress_posting(std::string namebase,
	std::ofstream& ofile, T ite, int positional){
	mData meta;
	meta.filename = namebase;
    meta.start_pos = ofile.tellp();

	unsigned int currID = 0;
	int postingCount = 1;
	std::string currTerm;
	//declare vectors to store 128 values TODO: use arrays instead of vectors
	std::vector<unsigned int> v_docID;
	std::vector<unsigned int> v_second;
	std::vector<unsigned int> v_third;
	//declare vectors that will be written to file
	std::vector<unsigned int> last_docID;
	std::vector<unsigned int> size_of_block;
	std::vector<uint8_t> docID_biv;
	std::vector<uint8_t> second_biv;
	std::vector<uint8_t> third_biv;
	//initialize compression method, 1: varbyte
	int doc_method = 1;
	int second_method = 1;
	int third_method = 1;
	if(positional){
		ite = positional_index.begin();
		T end = positional_index.end();
	}
	else{
		ite = nonpositional_index.begin();
		T end = nonpositional_index.end();
	}

 	while( ite != end ){
 		currID = ite->second->termID;
 		currTerm = ite->first;
		//writing metadata to file
		ofile.write(reinterpret_cast<const char *>(&currID), sizeof(currID));
		ofile.write(reinterpret_cast<const char *>(&doc_method), sizeof(doc_method));
		ofile.write(reinterpret_cast<const char *>(&second_method), sizeof(second_method));
		if(positional) ofile.write(reinterpret_cast<const char *>(&third_method), sizeof(third_method));
		meta.posting_offset = ofile.tellp();

 		while( ite->second->termID == currID && ite != end ){
			while( postingCount % (BLOCK+1) != 0 && ite->second->termID == currID && ite != end ){
				v_docID.push_back(ite->second->docID);
	 			v_second.push_back(ite->second->second);
	 			if(positional) v_third.push_back(ite->third);
	 			ite ++;
				postingCount ++;
			}
			//add last value of docID
			ite --;
			last_docID.push_back(ite->second->docID);
			ite ++;
			//compress block of 128
			docID_biv = compress_field(v_docID, doc_method, 1);
			if(positional){
				second_biv = compress_field(v_second, second_method, 1); //compress fragmentID in positional posting
				third_biv = compress_field(v_third, third_method, 1); //compress position in positional posting
			}else{
				second_biv = compress_field(v_second, second_method, 0); //compress frequency in nonpositional posting
			}
			//blocks of docID, followed by blocks of frequency
			write<uint8_t>(docID_biv, ofile);
			write<uint8_t>(second_biv, ofile);
			if(positional) write<uint8_t>(third_biv, ofile);

			v_docID.clear();
			v_second.clear();
			v_third.clear();
			docID_biv.clear();
			second_biv.clear();
			third_biv.clear();
 		}
		meta.postingCount_offset = ofile.tellp();
		ofile.write(reinterpret_cast<const char *>(&postingCount), sizeof(postingCount));
		write<unsigned int>(last_docID, ofile);
		meta.size_offset = ofile.tellp();
		write<unsigned int>(size_of_block, ofile);
		postingCount = 0;

		meta.end_offset = ofile.tellp();
		if(positional) exlex.addPositional(currTerm, meta);
 		else exlex.addNonPositional(currTerm, meta);
 	}
}

std::vector<uint8_t> Index::compress_field(std::vector<unsigned int>& field, int method, int delta){
    /* the first entry of every block is NOT the delta of the last entry from the previous one */
	std::vector<uint8_t> field_biv;
	if(method){
		if(delta){
			std::vector<unsigned int> delta;
			std::vector<unsigned int>::iterator it = field.begin();
			unsigned int prev = 0;
			while(it != field.end()){
				delta.push_back(*it - prev);
				prev = *it;
				it ++;
			}
		}
		else field_biv = VBEncode(field);
	}
	return field_biv;
}

void Index::decompress_p_posting(unsigned int termID, std::ifstream& ifile, std::string namebase){
    /* Decompress positional postings and store them in map structure */
    std::string filename = std::string(PDIR) + namebase;
    mData meta = getPositional(termID, namebase);

    int method1, method2, method3;

    filez.read(reinterpret_cast<char *>(&method1), sizeof(method1));
    filez.read(reinterpret_cast<char *>(&method2), sizeof(method2));
    filez.read(reinterpret_cast<char *>(&method3), sizeof(method3));

    ifile.seekg(meta.posting_offset);
    std::vector<unsigned int> docID, fragID, pos;
    int count;
    //read all the alternating blocks from compressed index and decompress
    int length = meta.postingCount_offset - meta.posting_offset;
    char* buffer = new char [length];
    ifile.read(buffer, length);
    std::vector<unsigned int> decompressed = VBDecode(buffer, length);
    std::vector<unsigned int>::iterator it = decompressed.begin();
    unsigned int prevID; //TODO 3: assume only doc ID is delta encoded, which needs modification of previous code

    while( it != decompressed.end() ){
        count = 0;
        prevID = 0;
        while( count < BLOCK ){
            docID.push_back( prevID + *it );
            prevID = *it;
            count ++;
            it ++;
        }
        count = 0;
        while( count < BLOCK ){
            fragID.push_back(*it);
            count ++;
            it ++;
        }
        count = 0;
        while( count < BLOCK ){
            pos.push_back(*it);
            count ++;
            it ++;
        }
    }
    std::vector<unsigned int>::iterator it1 = docID.begin();
    std::vector<unsigned int>::iterator it2 = fragID.begin();
    std::vector<unsigned int>::iterator it3 = pos.begin();
    Posting p;
    std::vector<Posting> postings;
    while( it1 != docID.end() ){
        p.termID = termID;
        p.docID = *it1;
        p.second = *it2;
        p.third = *it3;
        postings.push_back(p);
        it1 ++;
        it2 ++;
        it3 ++;
    }
    positional_index.insert( std::pair<unsigned int, std::vector<Posting>>(termID, postings) );

    return;
}

std::vector<char> Index::read_com(std::ifstream& infile, long end_pos){
	//read compressed forward index
	char c;
	std::vector<char> result;
	while(infile.tellg() != end_pos){
        infile.get(c);
		result.push_back(c);
	}
    //the last one is not read in the loop
    infile.get(c);
    result.push_back(c);
	return result;
}

void Index::merge_test(){
    /**
	 * Test if there are two files of same index number on disk.
	 * If there is, merge them and then call merge_test again until
	 * all index numbers has only one file each.
	 */
	int indexnum = 0;
	std::string dir = std::string(PDIR);
    std::string npdir = std::string(NPDIR);
	std::vector<std::string> files = read_directory(dir);
    std::string fp = std::string("I") + std::to_string(indexnum);
    std::string fnp = std::string("L") + std::to_string(indexnum);

	while(check_contain(files, fp)){
		//if In exists already, merge In with Zn
        files.clear();
		merge_p(indexnum);
		indexnum ++;
        fp = std::string("I") + std::to_string(indexnum);
        files.clear();
        files = read_directory(dir);
	}
    indexnum = 0;
    std::vector<std::string> npfiles = read_directory(npdir);
    while(check_contain(npfiles, fnp)){
		//if Ln exists already, merge In with Xn
        npfiles.clear();
		merge_np(indexnum);
		indexnum ++;
        fnp = std::string("L") + std::to_string(indexnum);
        npfiles.clear();
        npfiles = read_directory(npdir);
	}
}

void Index::merge(int indexnum, int positional){

	std::ifstream filez;
	std::ifstream filei;
	std::ofstream ofile;
	if(positional) std::string dir(PDIR);
	else std::string dir(NPDIR);

	//determine the name of the output file, if "Z" file exists, than compressed to "I" file.
    char flag = 'Z';
	filez.open(dir + "Z" + std::to_string(indexnum));
	filei.open(dir + "I" + std::to_string(indexnum));

    std::string namebase1 = "Z" + std::to_string(indexnum);
	std::string namebase2 = "I" + std::to_string(indexnum);
    std::string namebaseo;

	ofile.open(dir + "Z" + std::to_string(indexnum + 1), std::ios::app | std::ios::binary);
    namebaseo = flag + std::to_string(indexnum + 1);
	if(ofile.tellp() != 0){
		ofile.close();
		ofile.open(dir + "I" + std::to_string(indexnum + 1), std::ios::ate | std::ios::binary);
        flag = 'I';
        namebaseo = flag + std::to_string(indexnum + 1);
	}
    std::cout << "Merging into " << flag << indexnum + 1 << "------------------------------------" << std::endl;

	unsigned int termIDZ, termIDI;
    if( filez.is_open() && filei.is_open() ){
        while( !filez.eof() || !filei.eof() ){
    		filez.read(reinterpret_cast<char *>(&termIDZ), sizeof(termIDZ));
    		filei.read(reinterpret_cast<char *>(&termIDI), sizeof(termIDI));

    		if( termIDZ < termIDI ) //TODO 2.1: copy from start to end
    		else if( termIDI < termIDZ ) //TODO 2.2: copy from start to end
    		else if( termIDI == termIDZ ){
                /*
    			int doc_methodi, second_methodi, third_methodi,
    			doc_methodz, second_methodz, third_methodz;

    			filez.read(reinterpret_cast<char *>(&doc_methodz), sizeof(doc_methodz));
    			filez.read(reinterpret_cast<char *>(&second_methodz), sizeof(second_methodz));
    			filei.read(reinterpret_cast<char *>(&doc_methodi), sizeof(doc_methodi));
    			filei.read(reinterpret_cast<char *>(&second_methodi), sizeof(second_methodi));
    			if( positional ){
    				filez.read(reinterpret_cast<char *>(&third_methodz), sizeof(third_methodz));
    				filei.read(reinterpret_cast<char *>(&third_methodi), sizeof(doc_methodi));
                }
                */
                if( positional ){
                    decompress_p_posting(termID, filez, namebase1);
                    decompress_p_posting(termID, filei, namebase2);
                    P_ITE ite = positional_index.begin();
                    compress_posting(namebaseo, ofile, ite, 1);
                }
                else{
                    decompress_np_posting(termID, filez, filei, namebase1, namebase2);
                    NP_ITE ite = nonpositional_index.begin();
                    compress_posting(namebaseo, ofile, ite, 0);
                }
    		}
    	}
    }
    else std::cerr << "Error opening file." << endl;

	filez.close();
	filei.close();
	ofile.close();
    std::string filename1 = dir + "Z" + std::to_string(indexnum);
    std::string filename2 = dir + "I" + std::to_string(indexnum);
    //deleting two files
    if( remove( filename1.c_str() ) != 0 ) std::cout << "Error deleting file" << std::endl;
    if( remove( filename2.c_str() ) != 0 ) std::cout << "Error deleting file" << std::endl;
}

//TODO 1: finish this function
void Index::decompress_np_posting(unsigned int termID, ){
    /**
	 * Merge non-positional index.
	 */
	std::ifstream filez;
	std::ifstream filei;
	std::ofstream ofile;
    std::string pdir(NPDIR);
    char flag = 'X';//determine the name of the output file
	filez.open(pdir + "X" + std::to_string(indexnum));
	filei.open(pdir + "L" + std::to_string(indexnum));

	ofile.open(pdir + "X" + std::to_string(indexnum + 1), std::ios::app | std::ios::binary);
	if(ofile.tellp() != 0){
        std::cout << "cannot merge to " << flag << indexnum + 1 << std::endl;
		ofile.close();
		ofile.open(pdir + "L" + std::to_string(indexnum + 1), std::ios::ate | std::ios::binary);
        flag = 'L';
	}

    std::cout << "Merging into " << flag << indexnum + 1 << "------------------------------------" << std::endl;

	std::string file1 = "X" + std::to_string(indexnum);
	std::string file2 = "L" + std::to_string(indexnum);
	//std::vector<f_meta>& v1 = filemeta[file1];
	//std::vector<f_meta>& v2 = filemeta[file2];
	//std::vector<f_meta>::iterator it1 = v1.begin();
	//std::vector<f_meta>::iterator it2 = v2.begin();

	/**
	 * Go through the meta data of each file, do
	 * if there is a termID appearing in both, decode the part and merge
	 * else copy and paste the corresponding part of postinglist
	 * update the corresponding fileinfo of that termID
	 * assume that the posting of one term can be stored in memory
	 */

	 /*
	while( it1 != v1.end() && it2 != v2.end() ){
        //std::cout << it1->termID << ' ' << it2->termID << std::endl;
		if( it1->termID == it2->termID ){
			//decode and merge
			//update meta data corresponding to the term
			std::vector<nPosting> vp1 = decompress_np(file1, it1->termID);
            std::vector<nPosting> vp2 = decompress_np(file2, it2->termID);
			std::vector<nPosting> vpout; //store the sorted result

			//use NextGQ to write the sorted std::vector of Posting to disk
			std::vector<nPosting>::iterator vpit1 = vp1.begin();
			std::vector<nPosting>::iterator vpit2 = vp2.begin();
			while( vpit1 != vp1.end() && vpit2 != vp2.end() ){
				//NextGQ
				if( *vpit1 < *vpit2 ){
					vpout.push_back(*vpit1);
					vpit1 ++;
				}
				else if( *vpit1 > *vpit2 ){
					vpout.push_back(*vpit2);
					vpit2 ++;
				}
				else if ( *vpit1 == *vpit2 ){
                    vpout.push_back(*vpit1);
                    vpout.push_back(*vpit2);
					vpit1 ++;
                    vpit2 ++;
					break;
				}
			}
            while( vpit1 != vp1.end()){
                vpout.push_back(*vpit1);
                vpit1 ++;
            }
            while( vpit2 != vp2.end()){
                vpout.push_back(*vpit2);
                vpit2 ++;
            }
			compress_np(vpout, indexnum + 1, flag);
			it1 ++;
			it2 ++;
		}
		else if( it1->termID < it2->termID ){
			std::vector<nPosting> vp = decompress_np(file1, it1->termID);
            compress_np(vp, indexnum + 1, flag);
			it1 ++;
		}
		else if( it1->termID > it2->termID ){
            std::vector<nPosting> vp = decompress_np(file2, it2->termID);
            compress_np(vp, indexnum + 1, flag);
			it2 ++;
		}
	}
	*/
	/*
	while (it1 != v1.end() ){
        std::vector<nPosting> vp = decompress_np(file1, it1->termID);
        compress_np(vp, indexnum + 1, flag);
        it1 ++;
	}
	while (it2 != v2.end() ){
        std::vector<nPosting> vp = decompress_np(file2, it2->termID);
        compress_np(vp, indexnum + 1, flag);
        it2 ++;
	}
	*/
	filez.close();
	filei.close();
	ofile.close();
    std::string filename1 = pdir + "X" + std::to_string(indexnum);
    std::string filename2 = pdir + "L" + std::to_string(indexnum);

	/*

    for( std::vector<f_meta>::iterator it = filemeta[file1].begin(); it != filemeta[file1].end(); it ++){
        update_t_meta(it->termID, file1);
    }

    for( std::vector<f_meta>::iterator it = filemeta[file2].begin(); it != filemeta[file2].end(); it ++){
        update_t_meta(it->termID, file2);
    }
	*/

    //deleting two files
    if( remove( filename1.c_str() ) != 0 )
        std::cout << "Error deleting file" << std::endl;

    if( remove( filename2.c_str() ) != 0 )
        std::cout << "Error deleting file" << std::endl;
}
