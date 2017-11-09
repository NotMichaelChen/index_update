/* Methods related to compressing posting list */
#include <iostream>
#include <fstream>
#include <dirent.h>

#include "index.hpp"
#include "varbyte.hpp"

#define PDIR "./disk_index/positional/"//path to static positional index
#define NPDIR "./disk_index/non_positional/"//path to static non-positional index

std::vector<std::string> Index::read_directory( std::string path ){
    /* List all the files in a directory. */
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
	/* Write the compressed posting to file byte by byte. */
	for(typename std::vector<T>::iterator it = num.begin(); it != num.end(); it++){
		ofile.write(reinterpret_cast<const char*>(&(*it)), sizeof(*it));
	}
}

template <typename T>
void Index::compress_posting(std::string namebase,
	std::ofstream& ofile, T ite, T end, int positional){
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

 	while( ite != end ){
 		currID = ite->second.termID;
 		currTerm = ite->first;
		//writing metadata to file
		ofile.write(reinterpret_cast<const char *>(&currID), sizeof(currID));
		ofile.write(reinterpret_cast<const char *>(&doc_method), sizeof(doc_method));
		ofile.write(reinterpret_cast<const char *>(&second_method), sizeof(second_method));
		if(positional) ofile.write(reinterpret_cast<const char *>(&third_method), sizeof(third_method));
		meta.posting_offset = ofile.tellp();

 		while( ite->second.termID == currID && ite != end ){
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
				second_biv = compress_field(v_second, second_method, 0); //compress fragmentID in positional posting
				third_biv = compress_field(v_third, third_method, 0); //compress position in positional posting
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
		if(positional) exlex.addPositional(currID, meta);
 		else exlex.addNonPositional(currID, meta);
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
    mData meta = exlex.getPositional(termID, namebase);

    int method1, method2, method3;

    ifile.read(reinterpret_cast<char *>(&method1), sizeof(method1));
    ifile.read(reinterpret_cast<char *>(&method2), sizeof(method2));
    ifile.read(reinterpret_cast<char *>(&method3), sizeof(method3));

    ifile.seekg(meta.posting_offset);
    std::vector<unsigned int> docID, fragID, pos;
    int count;
    //read all the alternating blocks from compressed index and decompress
    int length = meta.postingCount_offset - meta.posting_offset;
    char* buffer = new char [length];
    ifile.read(buffer, length);
    std::vector<unsigned int> decompressed = VBDecode(buffer, length);
    delete[] buffer;
    std::vector<unsigned int>::iterator it = decompressed.begin();
    unsigned int prevID;
    //TODO: here assume only doc ID is delta encoded; the other two can also be delta encoded,
    //but need change code to identify doc and fragment boundary in file
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
    ifile.seekg(meta.end_offset);
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
    std::string fnp = std::string("I") + std::to_string(indexnum);

	while(check_contain(files, fp)){
		//if In exists already, merge In with Zn
        files.clear();
		merge(indexnum, 1);//merge positional
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
		merge(indexnum, 0);//merge non_positional
		indexnum ++;
        fnp = std::string("I") + std::to_string(indexnum);
        npfiles.clear();
        npfiles = read_directory(npdir);
	}
}

void Index::merge(int indexnum, int positional){

	std::ifstream filez;
	std::ifstream filei;
	std::ofstream ofile;
    std::string dir;
	if(positional) dir.assign(PDIR);
	else dir.assign(NPDIR);

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

    mData metai, metaz;
	unsigned int termIDZ, termIDI;
    if( filez.is_open() && filei.is_open() ){
        while( !filez.eof() || !filei.eof() ){
    		filez.read(reinterpret_cast<char *>(&termIDZ), sizeof(termIDZ));
    		filei.read(reinterpret_cast<char *>(&termIDI), sizeof(termIDI));

    		if( termIDZ < termIDI ){
                if( positional ) metaz = exlex.getPositional(termIDZ, namebase1);
                else metaz = exlex.getNonPositional(termIDZ, namebase1);
                int length = metaz.end_offset - metaz.start_pos;
                char* buffer = new char [length];
                filez.read(buffer, length);
                ofile.write(buffer, length);
                delete[] buffer;
            }
    		else if( termIDI < termIDZ ){
                if( positional ) metai = exlex.getPositional(termIDI, namebase2);
                else metai = exlex.getNonPositional(termIDZ, namebase2);
                int length = metai.end_offset - metai.start_pos;
                char* buffer = new char [length];
                filei.read(buffer, length);
                ofile.write(buffer, length);
                delete[] buffer;
            }
    		else if( termIDI == termIDZ ){
                if( positional ){
                    decompress_p_posting(termIDZ, filez, namebase1);
                    decompress_p_posting(termIDI, filei, namebase2);
                    P_ITE ite = positional_index.begin();
                    P_ITE end = positional_index.end();
                    compress_posting(namebaseo, ofile, ite, end, 1);
                }
                else{
                    decompress_np_posting(termIDI, filez, filei, namebase1, namebase2);
                    NP_ITE ite = nonpositional_index.begin();
                    NP_ITE end = nonpositional_index.end();
                    compress_posting(namebaseo, ofile, ite, end, 0);
                }
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

void Index::decompress_np_posting(unsigned int termID, std::ifstream& filez,
    std::ifstream& filei, std::string namebase1, std::string namebase2){
    int doc_methodi, second_methodi, doc_methodz, second_methodz;

    filez.read(reinterpret_cast<char *>(&doc_methodz), sizeof(doc_methodz));
    filez.read(reinterpret_cast<char *>(&second_methodz), sizeof(second_methodz));
    filei.read(reinterpret_cast<char *>(&doc_methodi), sizeof(doc_methodi));
    filei.read(reinterpret_cast<char *>(&second_methodi), sizeof(second_methodi));

    mData metaz = exlex.getNonPositional(termID, namebase1);
    mData metai = exlex.getNonPositional(termID, namebase2);

    filez.seekg(metaz.posting_offset);
    filei.seekg(metai.posting_offset);
    std::vector<unsigned int> docIDi, docIDz, freqi, freqz;
    int count;
    //read all the alternating blocks from compressed index and decompress
    int lengthz = metaz.postingCount_offset - metaz.posting_offset;
    int lengthi = metai.postingCount_offset - metai.posting_offset;
    char* bufferz = new char [lengthz];
    char* bufferi = new char [lengthi];
    filez.read(bufferz, lengthz);
    filei.read(bufferi, lengthi);
    std::vector<unsigned int> decomz = VBDecode(bufferz, lengthz);
    std::vector<unsigned int> decomi = VBDecode(bufferi, lengthi);
    delete[] bufferi;
    delete[] bufferz;
    std::vector<unsigned int>::iterator itz = decomz.begin();
    std::vector<unsigned int>::iterator iti = decomi.begin();
    unsigned int prevIDz, prevIDi;

    while( itz != decomz.end() ){
        count = 0;
        prevIDz = 0;
        while( count < BLOCK ){
            prevIDz = prevIDz + *itz;
            docIDz.push_back( prevIDz );
            count ++;
            itz ++;
        }
        count = 0;
        while( count < BLOCK ){
            freqz.push_back(*itz);
            count ++;
            itz ++;
        }
    }
    while( iti != decomi.end() ){
        count = 0;
        prevIDi = 0;
        while( count < BLOCK ){
            prevIDi = prevIDi + *iti;
            docIDi.push_back( prevIDi );
            count ++;
            iti ++;
        }
        count = 0;
        while( count < BLOCK ){
            freqi.push_back(*iti);
            count ++;
            iti ++;
        }
    }

    itz = docIDz.begin();
    iti = docIDi.begin();
    std::vector<unsigned int>::iterator itfz = freqz.begin();
    std::vector<unsigned int>::iterator itfi = freqi.begin();
    Posting p;
    std::vector<Posting> postings;
    while( itz != docIDz.end() && iti != docIDi.end() ){
        if( *itz > *iti ){
            p.docID = *itz;
            p.second = *itfz;
            itz ++;
            itfz ++;
            postings.push_back(p);
        }
        else if( *itz < *iti ){
            p.docID = *iti;
            p.second = *itfi;
            iti ++;
            itfi ++;
            postings.push_back(p);
        }
        else{
            //if equal, use the frequency of I file
            p.docID = *iti;
            p.second = *itfi;
            iti ++;
            itz ++;
            itfi ++;
            itfz ++;
            postings.push_back(p);
        }

    }
    if( itz != docIDz.end() ){
        p.docID = *itz;
        p.second = *itfz;
        itz ++;
        itfz ++;
        postings.push_back(p);
    }
    else if( iti != docIDi.end() ){
        p.docID = *iti;
        p.second = *itfi;
        iti ++;
        itfi ++;
        postings.push_back(p);
    }

    positional_index.insert( std::pair<unsigned int, std::vector<Posting>>(termID, postings) );
    filez.seekg(metaz.end_offset);
    filei.seekg(metai.end_offset);
}
