#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <string>
#include <cstdio>
#include <dirent.h>

#include "reader.hpp"
#include "posting.hpp"
#include "meta.hpp"
#include "compressor.hpp"
#include "strless.hpp"

#define NO_DOC 10 //temporary use
#define POSTING_LIMIT 100 //make sure doesn't exceed memory limit
#define INDEX "./test_data/compressedIndex"
#define INFO "./test_data/docInfo"
#define PDIR "./disk_index/positional/"

using namespace std;

bool strless::operator() (const string & first, const string & second ) const  {
    return first < second;
}

Posting::Posting (){}

Posting::~Posting (){}

Posting::Posting(unsigned int id, unsigned int d, unsigned int f, unsigned int p){
	termID = id;
	docID = d;
	fragID = f;
	pos = p;
}

bool operator< (const Posting& p1, const Posting& p2) {
	if(p1.termID == p2.termID){
		if(p1.docID == p2.docID){
			if(p1.fragID == p2.fragID){
				return (p1.pos < p2.pos);
			}else{
				return (p1.fragID < p2.fragID);
			}
		}else{
			return (p1.docID < p2.docID);
		}
	}else{
		return (p1.termID < p2.termID);
	}
}

bool operator> (const Posting& p1, const Posting& p2) {
    if(p1.termID == p2.termID){
		if(p1.docID == p2.docID){
			if(p1.fragID == p2.fragID){
				return (p1.pos > p2.pos);
			}else{
				return (p1.fragID > p2.fragID);
			}
		}else{
			return (p1.docID > p2.docID);
		}
	}else{
		return (p1.termID > p2.termID);
	}
}

bool operator== (const Posting& p1, const Posting& p2){
	if(p1.termID == p2.termID && p1.docID == p2.docID && p1.fragID == p2.fragID && p1.pos == p2.pos) return true;
	else return false;
}

struct less_than_key{
    inline bool operator() (const Posting& p1, const Posting& p2)
    {
        if(p1.termID == p2.termID){
        	if(p1.docID == p2.docID){
        		if(p1.fragID == p2.fragID){
        			return (p1.pos < p2.pos);
        		}else{
        			return (p1.fragID < p2.fragID);
        		}
        	}else{
        		return (p1.docID < p2.docID);
        	}
        }else{
        	return (p1.termID < p2.termID);
        }
    }
};

std::vector<std::string> Compressor::read_directory( std::string path ){
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
                string d_n(de->d_name);
      			result.push_back( d_n );
      		}
    		closedir( dp );
	    }
	return result;
}

void Compressor::update_f_meta(map<string, vector<f_meta>, strless>& filemeta, string s1, string s2){
	filemeta.erase(s1);
	filemeta.erase(s2);
}

void Compressor::update_t_meta(unsigned int termID, string file1, string file2, map<unsigned int, vector<mData>>& dict){
	//delete old metadata, compress_p will take care of adding new
	vector<mData>& metavec= dict[termID];
	for( vector<mData>::iterator it = metavec.begin(); it != metavec.end(); it++){
		if( it->filename == file1 ) metavec.erase(it);
		else if( it->filename == file2 ) metavec.erase(it);
	}
}

void Compressor::copy_and_paste(ifstream& ifile, ofstream& ofile, long start, long end){
	ifile.seekg(start);
	char c;
	while(ifile.tellg() != end){
		ifile.get(c);
		ofile << c;
	}
    //cout << "Copy and pasting finished." << endl;
}

void Compressor::merge(map<string, vector<f_meta>, strless>& filemeta, int indexnum, map<unsigned int, vector<mData>>& dict){
    cout << "Merging index " << indexnum << endl;
	ifstream filez;
	ifstream filei;
	ofstream ofile;
    string pdir(PDIR);
    char flag = 'Z';//determine the name of the output file
	filez.open(pdir + "Z" + to_string(indexnum));
	filei.open(pdir + "I" + to_string(indexnum));

	ofile.open(pdir + "Z" + to_string(indexnum + 1), ios::app | ios::binary);
	if(ofile.tellp() != 0){
		ofile.close();
		ofile.open(pdir + "I0", ios::ate | ios::binary);
        flag = 'I';
	}

	long ostart;
	long oend;
	string file1 = "Z" + to_string(indexnum);
	string file2 = "I" + to_string(indexnum);
	vector<f_meta>& v1 = filemeta[file1];
	vector<f_meta>& v2 = filemeta[file2];
	vector<f_meta>::iterator it1 = v1.begin();
	vector<f_meta>::iterator it2 = v2.begin();

	//Go through the meta data of each file, do
	//if there is a termID appearing in both, decode the part and merge
	//else copy and paste the corresponding part of postinglist
	//update the corresponding fileinfo of that termID
	//assume that the posting of one term can be stored in memory
	while( it1 != v1.end() && it2 != v2.end() ){
        cout << it1->termID << ' ' << it2->termID << endl;
		if( it1->termID == it2->termID ){
			//decode and merge
			//update meta data corresponding to the term
			vector<Posting> vp1 = Reader::decompress(file1, it1->termID, dict);
            vector<Posting> vp2 = Reader::decompress(file2, it2->termID, dict);
/*
            for( vector<Posting>::iterator it = vp1.begin(); it != vp1.end(); it ++){
                cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
            }
            cout << endl;
            for( vector<Posting>::iterator it = vp2.begin(); it != vp2.end(); it ++){
                cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
            }
            cout << endl;
*/
			vector<Posting> vpout; //store the sorted result

			//use NextGQ to write the sorted vector of Posting to disk
			vector<Posting>::iterator vpit1 = vp1.begin();
			vector<Posting>::iterator vpit2 = vp2.begin();
			while( vpit1 != vp1.end() && vpit2 != vp2.end() ){
				//NextGQ
				if( *vpit1 < *vpit2 ){
                    //cout << vpit1->termID << ' ' << vpit1->docID << ' ' << vpit1->fragID << ' ' << vpit1->pos << endl;
                    //cout << vpit2->termID << ' ' << vpit2->docID << ' ' << vpit2->fragID << ' ' << vpit2->pos << endl;

					vpout.push_back(*vpit1);
					vpit1 ++;
				}
				else if( *vpit1 > *vpit2 ){
					vpout.push_back(*vpit2);
					vpit2 ++;
				}
				else if ( *vpit1 == *vpit2 ){
					cout << "Error: same posting appearing in different indexes." << endl;
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
/*
            for( vector<Posting>::iterator it = vpout.begin(); it != vpout.end(); it ++){
                cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
            }
*/
			update_t_meta(it1->termID, file1, file2, dict);
			compress_p(vpout, filemeta, dict, indexnum + 1, flag);
			it1 ++;
			it2 ++;
		}
		else if( it1->termID < it2->termID ){
			copy_and_paste(filez, ofile, it1->start_pos, it1->end_pos);
			it1 ++;
		}
		else if( it1->termID > it2->termID ){
			copy_and_paste(filei, ofile, it2->start_pos, it2->end_pos);
			it2 ++;
		}
	}
    cout << "NextGQ finished." << endl;

	//copy and paste the unfinished index
	if (it1 != v1.end() ){
        cout << "Filez not finished." << endl;
        filez.seekg(it1->start_pos);
    	char c;
    	while(filez.get(c)){
    		ofile << c;
    	}
	}
	if (it2 != v2.end() ){
        cout << "Filei not finished." << endl;
        filez.seekg(it2->start_pos);
    	char c;
    	while(filez.get(c)){
    		ofile << c;
    	}
	}
	//update_f_meta(filemeta, pdir + "Z" + to_string(indexnum), pdir + "I" + to_string(indexnum));
	filez.close();
	filei.close();
	ofile.close();
    string filename1 = pdir + "Z" + to_string(indexnum);
    string filename2 = pdir + "I" + to_string(indexnum);

    //deleting two files
    if( remove( filename1.c_str() ) != 0 )
        cout << "Error deleting file" << endl;
    else
        cout << "File successfully deleted" << endl;

    if( remove( filename2.c_str() ) != 0 )
        cout << "Error deleting file" << endl;
    else
        cout << "File successfully deleted" << endl;
}

bool Compressor::check_contain(vector<string> v, string f){
    for( vector<string>::iterator it = v.begin(); it != v.end(); it ++){
        if( *it == f) return true;
    }
    return false;
}

void Compressor::merge_test(map<string, vector<f_meta>, strless>& filemeta, map<unsigned int, vector<mData>>& dict){
    cout << "Merge testing " << endl;
	int indexnum = 0;
	string dir = string(PDIR);
	vector<string> files = read_directory(dir);
    string f = string("I") + to_string(indexnum);

	while(check_contain(files, f)){
		//if In exists already, merge In with Zn
        files.clear();
		merge(filemeta, indexnum, dict);
		indexnum ++;
        f = string("I") + to_string(indexnum);
        vector<string> files = read_directory(dir);
	}
}

void Compressor::write(vector<uint8_t> num, ofstream& ofile){
	for(vector<uint8_t>::iterator it = num.begin(); it != num.end(); it++){
		ofile.write(reinterpret_cast<const char*>(&(*it)), 1);
	}
}

std::vector<char> Compressor::read_com(ifstream& infile){
	char c;
	vector<char> result;
	while(infile.get(c)){
		result.push_back(c);
	}
	return result;
}

std::vector<uint8_t> Compressor::VBEncode(unsigned int num){
	vector<uint8_t> result;
	uint8_t b;
	while(num >= 128){
		int a = num % 128;
		bitset<8> byte(a);
		byte.flip(7);
		num = (num - a) / 128;
		b = byte.to_ulong();
		result.push_back(b);
	}
	int a = num % 128;
	bitset<8> byte(a);
	b = byte.to_ulong();
	result.push_back(b);
	return result;
}

std::vector<uint8_t> Compressor::VBEncode(vector<unsigned int>& nums){
	vector<uint8_t> biv;
	vector<uint8_t> result;
	for( vector<unsigned int>::iterator it = nums.begin(); it != nums.end(); it ++){
		biv = VBEncode(*it);
		result.insert(result.end(), biv.begin(), biv.end());
	}
	return result;
}

vector<uint8_t> Compressor::compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv, vector<unsigned int> &last_id_biv){
	if(method){
		std::vector<unsigned int> block;
		std::vector<unsigned int>::iterator it = field.begin();
		std::vector<uint8_t> field_biv;
		std::vector<uint8_t> biv;

		unsigned int prev = 0;
		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();

			while(size_block < 64 && it != field.end()){
				block.push_back(*it - prev);
				prev = *it;
				size_block ++;
				it ++;
			}
			biv = VBEncode(block);
			last_id_biv.push_back(prev);//the last element of every block needs to be stored
			field_biv.insert(field_biv.end(), biv.begin(), biv.end());
			meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
		}
		return field_biv;
	}
}

vector<uint8_t> Compressor::compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv){
	if(method){
		std::vector<unsigned int> block;
		std::vector<unsigned int>::iterator it = field.begin();
		std::vector<uint8_t> field_biv;
		std::vector<uint8_t> biv;


		int prev;
		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();
			prev = 0;//the first element of every block needs to be renumbered

			while(size_block < 64 && it != field.end()){
				block.push_back(*it - prev);
				prev = *it;
				size_block ++;
				it ++;
			}
			biv = VBEncode(block);

			field_biv.insert(field_biv.end(), biv.begin(), biv.end());
			meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
		}
		return field_biv;
	}
}

mData Compressor::compress_p(string namebase, ofstream& ofile, f_meta& fm, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_fragID, std::vector<unsigned int>& v_pos){
    string filename = string(PDIR) + namebase;

	std::vector<unsigned int> v_last_id;
	std::vector<uint8_t> docID_biv;
	std::vector<uint8_t> fragID_biv;
	std::vector<uint8_t> pos_biv;

	std::vector<uint8_t> last_id_biv;
	std::vector<uint8_t> size_doc_biv;
	std::vector<uint8_t> size_frag_biv;
	std::vector<uint8_t> size_pos_biv;

	docID_biv = compress(v_docID, 1, 1, size_doc_biv, v_last_id);
	last_id_biv = VBEncode(v_last_id);

	fragID_biv = compress(v_fragID, 1, 0, size_frag_biv);
	pos_biv = compress(v_pos, 1, 0, size_pos_biv);

	mData meta;
    meta.filename = namebase;

	fm.start_pos = ofile.tellp();
    meta.start_pos = ofile.tellp();
	write(last_id_biv, ofile);

	meta.meta_doc_start = ofile.tellp();
	write(size_doc_biv, ofile);

	meta.meta_frag_start = ofile.tellp();
	write(size_frag_biv, ofile);

	meta.meta_pos_start = ofile.tellp();
	write(size_pos_biv, ofile);

	meta.posting_start = ofile.tellp();
	write(docID_biv, ofile);

	meta.frag_start = ofile.tellp();
	write(fragID_biv, ofile);

	meta.pos_start = ofile.tellp();
	write(pos_biv, ofile);

	fm.end_pos = ofile.tellp();
    meta.end_pos = ofile.tellp();

	return meta;
}

void Compressor::compress_p(std::vector<Posting>& pList, std::map<string, vector<f_meta>, strless>& filemeta, map<unsigned int, vector<mData>>& dict, int indexnum, char prefix){
	//pass in forward index of same termID
	//compress positional index
	ofstream ofile;//positional inverted index
    string pdir(PDIR);
    string namebase;
    string filename;
    if(prefix == 'a'){
        filename = pdir + "Z" + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);

    	if(ofile.tellp() != 0){
            cout << filename << " already exists." << endl;
    		ofile.close();
    		filename = pdir + "I" + to_string(indexnum);
            ofile.open(filename, ios::ate | ios::binary);
            namebase = string("I") + to_string(indexnum);
    	}else{
            namebase = string("Z") + to_string(indexnum);
        }
    }else{
        filename = pdir + prefix + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);
        namebase = string("Z") + to_string(indexnum);
    }

    if (ofile.is_open()){
        cout << "Compressing and writing to " << namebase << endl;
/*
        for( vector<Posting>::iterator it = pList.begin(); it != pList.end(); it ++){
            cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
        }
        cout << "end of plist" << endl;
*/

        std::vector<unsigned int> v_docID;
    	std::vector<unsigned int> v_fragID;
    	std::vector<unsigned int> v_pos;
    	mData mmData;
    	f_meta fm;
    	unsigned int num_of_p = 0;//number of posting of a certain term

    	unsigned int currID = pList[0].termID;//the ID of the term that is currently processing
        vector<Posting>::iterator it = pList.begin();
    	while( it != pList.end() ){
            //cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
    		while(it->termID == currID && it != pList.end()){
    			v_docID.push_back(it->docID);
    			v_fragID.push_back(it->fragID);
    			v_pos.push_back(it->pos);
    			it ++;
    			num_of_p ++;
    		}
    		fm.termID = currID;
            //cout << "Current term " << currID << endl;
    		mmData = compress_p(namebase, ofile, fm, v_docID, v_fragID, v_pos);
    		mmData.num_posting = num_of_p;
    		filemeta[namebase].push_back(fm);

    		//add mmdata to the dictionary of corresponding term
    		dict[currID].push_back(mmData);
            currID = it->termID;

    		num_of_p = 0;
    		v_docID.clear();
    		v_fragID.clear();
    		v_pos.clear();
    	}

    	ofile.close();

        //merge_test(filemeta, dict);//see if need to merge
    }else{
        cerr << "File cannot be opened." << endl;
    }
}

void Compressor::start_compress(map<string, vector<f_meta>, strless>& filemeta, map<unsigned int, vector<mData>>& dict){
	vector<Posting> invert_index;
	ifstream index;
	ifstream info;
	index.open(INDEX);
	info.open(INFO);
	string line;
	string value;
	vector<string> vec;
	char c;
	int p;
	int num;

	for(int i = 0; i < NO_DOC; i ++){
		vec.clear();
		getline(info, line);//read docInfo
		stringstream lineStream(line);
		unsigned int pos = 0;

		while(lineStream >> value){
			vec.push_back(value);
		}

		index.seekg(stoi(vec[2]));
		while(index.tellg() != (stoi(vec[2]) + stoi(vec[3]))){
			//for every document, do
			index.get(c);
			bitset<8> byte(c);
			num = 0; // store decoding termID
			p = 0;//power
			while(byte[7] == 1){
				byte.flip(7);
				num += byte.to_ulong()*pow(128, p);
				p++;
				index.get(c);
				byte = bitset<8>(c);
			}
			num += (byte.to_ulong())*pow(128, p);
			pos ++;

			Posting p(num, stoul(vec[1]), 0, pos);
			invert_index.push_back(p);
			if (invert_index.size() == POSTING_LIMIT){ // make sure doesn't exceed memory
				std::sort(invert_index.begin(), invert_index.end(), less_than_key());
				compress_p(invert_index, filemeta, dict);
                merge_test(filemeta, dict);//see if need to merge
                cout << "Exceeds" << endl;
				invert_index.clear();
			}
		}

	}
    std::sort(invert_index.begin(), invert_index.end(), less_than_key());
    compress_p(invert_index, filemeta, dict);
    merge_test(filemeta, dict);//see if need to merge
    cout << "Last bunch" << endl;
    invert_index.clear();

	index.close();
	info.close();
}

std::vector<char> Reader::read_com(ifstream& infile, long end_pos){//read compressed forward index
	char c;
	vector<char> result;
	while(infile.tellg() != end_pos){
        infile.get(c);
		result.push_back(c);
	}
    //the last one is not read in the loop
    infile.get(c);
    result.push_back(c);
	return result;
}

std::vector<unsigned int> Reader::VBDecode(ifstream& ifile, long start_pos, long end_pos){//ios::ate
	ifile.seekg(start_pos);
	char c;
	unsigned int num;
	int p;
	vector<unsigned int> result;
    if(end_pos == 0) end_pos = ifile.end;
	vector<char> vec = read_com(ifile, end_pos);

	for(vector<char>::iterator it = vec.begin(); it != vec.end(); it++){
		c = *it;
		bitset<8> byte(c);
		num = 0;
		p = 0;
		while(byte[7] == 1){
			byte.flip(7);
			num += byte.to_ulong()*pow(128, p);
			p++;
			it ++;
			c = *it;
			byte = bitset<8>(c);
		}
		num += (byte.to_ulong())*pow(128, p);

		result.push_back(num);
	}
	return result;
}

std::vector<unsigned int> Reader::VBDecode(vector<char>& vec){
	unsigned int num;
	vector<unsigned int> result;
	char c;
	int p;
	for(vector<char>::iterator it = vec.begin(); it != vec.end(); it++){
		c = *it;
		bitset<8> byte(c);
		num = 0;
		p = 0;
		while(byte[7] == 1){
			byte.flip(7);
			num += byte.to_ulong()*pow(128, p);
			p++;
			it ++;
			c = *it;
			byte = bitset<8>(c);
		}
		num += (byte.to_ulong())*pow(128, p);

		result.push_back(num);
	}
	return result;
}

std::vector<Posting> Reader::decompress(string namebase, unsigned int termID, map<unsigned int, vector<mData>>& dict){

	ifstream ifile;
    string filename = string(PDIR) + namebase;
	ifile.open(filename, ios::binary);
    vector<Posting> result;

    if(ifile.is_open()){
        cout << namebase << " Opened for Decompressing" << endl;
    	char c;
    	vector<char> readin;

    	vector<unsigned int> docID;
    	vector<unsigned int> fragID;
    	vector<unsigned int> pos;

        vector<mData>& mvec = dict[termID];
        vector<mData>::iterator currMVec;

        for( currMVec = mvec.begin(); currMVec != mvec.end(); currMVec ++){

            if( currMVec->filename == namebase ) {
                cout << currMVec->filename << endl;
                break;
            }
        }
        cout << "Decompressing " << endl;

    	ifile.seekg(currMVec->posting_start);
    	while(ifile.tellg() != currMVec->frag_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        docID = VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->frag_start);
    	while(ifile.tellg() != currMVec->pos_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        fragID = VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->pos_start);
    	while(ifile.tellg() != currMVec->end_pos){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        pos = VBDecode(readin);
        readin.clear();

    	vector<unsigned int>::iterator itdoc = docID.begin();
    	vector<unsigned int>::iterator itfrag = fragID.begin();
    	vector<unsigned int>::iterator itpos = pos.begin();

        unsigned int prevdoc =0;
        unsigned int prevfrag = 0;
        unsigned int prevpos = 0;

        int posting_num = 0;

    	while(itdoc != docID.end()){
            if(posting_num == 64){
                //keep track of block
                prevfrag = 0;
                prevpos = 0;
                posting_num = 0;
            }

            Posting p(termID, (*itdoc + prevdoc), (prevfrag+*itfrag), (prevpos+*itpos));
            //cout << termID << ' ' <<*itdoc << ' '<< *itfrag << ' ' << *itpos << endl;
            prevdoc = *itdoc + prevdoc;
            prevfrag = *itfrag + prevfrag;
            prevpos = *itpos + prevpos;
            result.push_back(p);
            itdoc ++;
            itfrag ++;
            itpos ++;
            posting_num ++;

    	}
    }else{
        cerr << "File cannot be opened." << endl;
    }

    cout << "Decompressing Finished ... " << endl;
    return result;
}

class Querior{
    void query( std::string terms  ){
    }
};

int main(){
	Compressor comp;
    map<string, unsigned int> lexical;
	map<unsigned int, vector<mData>> dict;
	map<string, vector<f_meta>, strless> filemeta;
	comp.start_compress(filemeta, dict);
    /*
    for( map<unsigned int, vector<mData>>::iterator it = dict.begin(); it != dict.end(); it ++){
        cout << it->first << endl;
        vector<mData> vec = it->second;
        for( vector<mData>::iterator ite = vec.begin(); ite != vec.end(); ite ++){
            cout << ite->filename << ' ' << ite->start_pos << endl;
        }
        cout << endl;
    }

    for( map<string, vector<f_meta>, strless>::iterator it = filemeta.begin(); it != filemeta.end(); it++){
        cout << it->first << endl;
        vector<f_meta> vec = it->second;
        for( vector<f_meta>::iterator ite = vec.begin(); ite != vec.end(); ite++){
            cout << ite->termID << ' ';
        }
        cout << endl;
    }
    */

    /*Querior q;
    string str;
    cin >> str;
    q.query(str, lexical);
    */

	return 0;
}
