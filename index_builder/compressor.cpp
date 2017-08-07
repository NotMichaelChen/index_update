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
#include <utility>

#include "reader.hpp"
#include "posting.hpp"
#include "meta.hpp"
#include "strless.hpp"
#include "compressor.hpp"
#include "comparison.hpp"

#define NO_DOC 10 //temporary use
#define POSTING_LIMIT 500 //make sure doesn't exceed memory limit
#define INDEX "./test_data/compressedIndex"
#define INFO "./test_data/docInfo"
#define PDIR "./disk_index/positional/"
#define NPDIR "./disk_index/non_positional/"

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

void Compressor::update_t_meta(unsigned int termID, string file, map<unsigned int, std::pair<vector<mData>, vector<mDatanp>>>& dict){
	//delete old metadata, compress_p will take care of adding new
	//vector<mData>& metavec= dict[termID];
	for( vector<mData>::iterator it = dict[termID].first.begin(); it != dict[termID].first.end(); it++){
		if( it->filename == file ) {
            dict[termID].first.erase(it);
            break;
        }
	}

    for( vector<mDatanp>::iterator it = dict[termID].second.begin(); it != dict[termID].second.end(); it++){
		if( it->filename == file ) {
            dict[termID].second.erase(it);
            break;
        }
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

void Compressor::merge_p(map<string, vector<f_meta>, strless>& filemeta, int indexnum, map<unsigned int, std::pair<vector<mData>, vector<mDatanp>>>& dict){
    //cout << "Merging index " << indexnum << endl;
	ifstream filez;
	ifstream filei;
	ofstream ofile;
    string pdir(PDIR);
    Reader r;
    char flag = 'Z';//determine the name of the output file
	filez.open(pdir + "Z" + to_string(indexnum));
	filei.open(pdir + "I" + to_string(indexnum));

	ofile.open(pdir + "Z" + to_string(indexnum + 1), ios::app | ios::binary);
	if(ofile.tellp() != 0){
		ofile.close();
		ofile.open(pdir + "I" + to_string(indexnum + 1), ios::ate | ios::binary);
        flag = 'I';
	}

    cout << "Merging into " << flag << indexnum + 1 << "------------------------------------" << endl;

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
        //cout << it1->termID << ' ' << it2->termID << endl; //TODO
		if( it1->termID == it2->termID ){
			//decode and merge
			//update meta data corresponding to the term
			vector<Posting> vp1 = r.decompress_p(file1, it1->termID, dict);
            vector<Posting> vp2 = r.decompress_p(file2, it2->termID, dict);
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
			//update_t_meta(it1->termID, file1, dict);
            //update_t_meta(it2->termID, file2, dict);
			compress_p(vpout, filemeta, dict, indexnum + 1, flag);
			it1 ++;
			it2 ++;
		}
		else if( it1->termID < it2->termID ){
			vector<Posting> vp = r.decompress_p(file1, it1->termID, dict);
            //update_t_meta(it1->termID, file1, dict);
            compress_p(vp, filemeta, dict, indexnum + 1, flag);
			it1 ++;
		}
		else if( it1->termID > it2->termID ){
            vector<Posting> vp = r.decompress_p(file2, it2->termID, dict);
            //update_t_meta(it2->termID, file2, dict);
            compress_p(vp, filemeta, dict, indexnum + 1, flag);
			it2 ++;
		}
	}

    //decompress from the old index and then compress to the new one to update metadata is time-consuming
    //need to find a more efficient way to update metadata while tranfering positngs
	while (it1 != v1.end() ){
        vector<Posting> vp = r.decompress_p(file1, it1->termID, dict);
        //update_t_meta(it1->termID, file1, dict);
        compress_p(vp, filemeta, dict, indexnum + 1, flag);
        it1 ++;
	}
	while (it2 != v2.end() ){
        vector<Posting> vp = r.decompress_p(file2, it2->termID, dict);
        //update_t_meta(it2->termID, file2, dict);
        compress_p(vp, filemeta, dict, indexnum + 1, flag);
        it2 ++;
	}

	filez.close();
	filei.close();
	ofile.close();
    string filename1 = pdir + "Z" + to_string(indexnum);
    string filename2 = pdir + "I" + to_string(indexnum);

    for( vector<f_meta>::iterator it = filemeta[file1].begin(); it != filemeta[file1].end(); it ++){
        update_t_meta(it->termID, file1, dict);
    }

    for( vector<f_meta>::iterator it = filemeta[file2].begin(); it != filemeta[file2].end(); it ++){
        update_t_meta(it->termID, file2, dict);
    }

    update_f_meta(filemeta, file1, file2);

    for( map<string, vector<f_meta>, strless>::iterator it = filemeta.begin(); it != filemeta.end(); it++){
        //cout << it->first << endl;
        vector<f_meta> vec = it->second;
        for( vector<f_meta>::iterator ite = vec.begin(); ite != vec.end(); ite++){
            //cout << ite->termID << ' ';
        }
        //cout << endl;
    }

    //deleting two files

    if( remove( filename1.c_str() ) != 0 )
        cout << "Error deleting file" << endl;

    if( remove( filename2.c_str() ) != 0 )
        cout << "Error deleting file" << endl;
}

bool Compressor::check_contain(vector<string> v, string f){
    for( vector<string>::iterator it = v.begin(); it != v.end(); it ++){
        if( *it == f) return true;
    }
    return false;
}

void Compressor::merge_test(map<string, vector<f_meta>, strless>& filemeta, map<unsigned int, std::pair<vector<mData>, vector<mDatanp>>>& dict){
    //cout << "Merge testing " << endl;
	int indexnum = 0;
	string dir = string(PDIR);
    string npdir = string(NPDIR);
	vector<string> files = read_directory(dir);
    string fp = string("I") + to_string(indexnum);
    string fnp = string("L") + to_string(indexnum);

	while(check_contain(files, fp)){
		//if In exists already, merge In with Zn
        files.clear();
		merge_p(filemeta, indexnum, dict);
		indexnum ++;
        fp = string("I") + to_string(indexnum);
        files.clear();
        files = read_directory(dir);
	}
    indexnum = 0;
    vector<string> npfiles = read_directory(npdir);
    while(check_contain(npfiles, fnp)){
		//if In exists already, merge In with Zn
        npfiles.clear();
		merge_np(filemeta, indexnum, dict);
		indexnum ++;
        fnp = string("L") + to_string(indexnum);
        npfiles.clear();
        npfiles = read_directory(npdir);
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

void Compressor::compress_p(std::vector<Posting>& pList, std::map<string, vector<f_meta>, strless>& filemeta, map<unsigned int, std::pair<vector<mData>, vector<mDatanp>>>& dict, int indexnum, char prefix){
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
        namebase = prefix+ to_string(indexnum);
    }

    if (ofile.is_open()){
        //cout << "Compressing and writing to " << namebase << endl;

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
            //cout << currID << ' '<< mmData.filename << endl;
    		dict[currID].first.push_back(mmData);
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

void Compressor::start_compress(map<string, vector<f_meta>, strless>& filemeta, map<unsigned int, std::pair<vector<mData>, vector<mDatanp>>>& dict){
	vector<Posting> p_index;
    vector<nPosting> np_index;
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
			p_index.push_back(p);


			if (p_index.size() == POSTING_LIMIT){ // make sure doesn't exceed memory

                cout << "Memory limit reaches." << endl;
                //cin.get();
				std::sort(p_index.begin(), p_index.end());//, less_than_key()
                //generate np_index
                int prevTermID = p_index[0].termID;
                int prevDocID = p_index[0].docID;
                unsigned int freq = 0;
                vector<Posting>::iterator it = p_index.begin();
                while(it != p_index.end()){
                    while( it->termID == prevTermID && it != p_index.end()){
                        while( it->docID == prevDocID && it != p_index.end()){
                            freq++;
                            it ++;
                        }
                        nPosting p(prevTermID, prevDocID, freq);
                        cout << prevTermID << ' '<< prevDocID << ' ' << freq << endl;
                        freq = 0;
                        prevDocID = it->docID;
                        np_index.push_back(p);
                    }
                    if(it!=p_index.end()){
                        prevTermID = it->termID;
                    }
                    else break;
                }
                compress_np(np_index, filemeta, dict);
				compress_p(p_index, filemeta, dict);
                merge_test(filemeta, dict);//see if need to merge
				p_index.clear();
                np_index.clear();
			}
		}

	}
    cout << "Last bunch " << p_index.size() << endl;
    std::sort(p_index.begin(), p_index.end());//, less_than_key()
    int prevTermID = p_index[0].termID;
    int prevDocID = p_index[0].docID;
    unsigned int freq = 0;
    vector<Posting>::iterator it = p_index.begin();
    while(it != p_index.end()){
        while( it->termID == prevTermID && it != p_index.end()){
            while( it->docID == prevDocID && it != p_index.end()){
                freq++;
                it ++;
            }
            nPosting p(prevTermID, prevDocID, freq);
            np_index.push_back(p);
            freq = 0;
            if(it != p_index.end())
                prevDocID = it->docID;
        }
        if(it!=p_index.end())
            prevTermID = it->termID;
        else break;
    }
    compress_np(np_index, filemeta, dict);
    compress_p(p_index, filemeta, dict);
    merge_test(filemeta, dict);//see if need to merge
    p_index.clear();
    np_index.clear();

	index.close();
	info.close();
}

vector<uint8_t> Compressor::compress_freq(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv){
    //not using delta coding
	if(method){
		std::vector<unsigned int> block;
		std::vector<unsigned int>::iterator it = field.begin();
		std::vector<uint8_t> field_biv;
		std::vector<uint8_t> biv;

		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();

			while(size_block < 64 && it != field.end()){
                //cout <<"Freq " <<  *it << endl;
				block.push_back(*it);
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

mDatanp Compressor::compress_np(string namebase, ofstream& ofile, f_meta& fm, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_freq, std::vector<unsigned int>& v_sign){
    string filename = string(NPDIR) + namebase;

	std::vector<unsigned int> v_last_id;
	std::vector<uint8_t> docID_biv;
	std::vector<uint8_t> freq_biv;
    std::vector<uint8_t> sign_biv;

	std::vector<uint8_t> last_id_biv;
	std::vector<uint8_t> size_doc_biv;
	std::vector<uint8_t> size_freq_biv;

	docID_biv = compress(v_docID, 1, 1, size_doc_biv, v_last_id);
	last_id_biv = VBEncode(v_last_id);

	freq_biv = compress_freq(v_freq, 1, 0, size_freq_biv);

    //compress sign vector
    vector<unsigned int>::iterator it = v_sign.begin();
    int num;
    uint8_t b;
    bitset<8> byte;
    while( it != v_sign.end() ){
        num = 0;
        while(num != 8 && it != v_sign.end()){
            if( *it == 1 )
                byte.flip( num );
            num ++;
            it ++;
        }
        b = byte.to_ulong();
        sign_biv.push_back(b);
    }

	mDatanp meta;
    meta.filename = namebase;

	fm.start_pos = ofile.tellp();
    meta.start_pos = ofile.tellp();
	write(last_id_biv, ofile);

	meta.meta_doc_start = ofile.tellp();
	write(size_doc_biv, ofile);

	meta.meta_freq_start = ofile.tellp();
	write(size_freq_biv, ofile);

	meta.posting_start = ofile.tellp();
	write(docID_biv, ofile);

	meta.freq_start = ofile.tellp();
	write(freq_biv, ofile);

	meta.sign_start = ofile.tellp();
	write(sign_biv, ofile);

	fm.end_pos = ofile.tellp();
    meta.end_pos = ofile.tellp();

	return meta;
}

void Compressor::compress_np(std::vector<nPosting>& npList, std::map<string, vector<f_meta>, strless>& filemeta, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict, int indexnum, char prefix){
    ofstream ofile;//positional inverted index
    string pdir(NPDIR);
    string namebase;
    string filename;
    if(prefix == 'a'){
        filename = pdir + "X" + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);

    	if(ofile.tellp() != 0){
            cout << filename << " already exists." << endl;
    		ofile.close();
    		filename = pdir + "L" + to_string(indexnum);
            ofile.open(filename, ios::ate | ios::binary);
            namebase = string("L") + to_string(indexnum);
    	}else{
            namebase = string("X") + to_string(indexnum);
        }
    }else{
        filename = pdir + prefix + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);
        namebase = prefix+ to_string(indexnum);
    }

    if (ofile.is_open()){
        //cout << "Compressing and writing to " << namebase << endl;

        std::vector<unsigned int> v_docID;
    	std::vector<unsigned int> v_freq;
        std::vector<unsigned int> v_sign;
    	mDatanp mmDatanp;
    	f_meta fm;

    	unsigned int currID = npList[0].termID;//the ID of the term that is currently processing
        vector<nPosting>::iterator it = npList.begin();
    	while( it != npList.end() ){
            //cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
    		while(it->termID == currID && it != npList.end()){
    			v_docID.push_back(it->docID);
    			v_freq.push_back(it->freq);
                v_sign.push_back(it->sign);
    			it ++;
    		}
    		fm.termID = currID;
            //cout << "Current term " << currID << endl;
    		mmDatanp = compress_np(namebase, ofile, fm, v_docID, v_freq, v_sign);
    		filemeta[namebase].push_back(fm);
            //cout << "This file contains: " << fm.termID << endl;

    		//add mmdata to the dictionary of corresponding term
            //cout << currID << ' '<< mmData.filename << endl;
    		dict[currID].second.push_back(mmDatanp);
            currID = it->termID;

    		v_docID.clear();
    		v_freq.clear();
    		v_sign.clear();
    	}

    	ofile.close();

        //merge_test(filemeta, dict);//see if need to merge
    }else{
        cerr << "File cannot be opened." << endl;
    }
}

void Compressor::merge_np(map<string, vector<f_meta>, strless>& filemeta, int indexnum, map<unsigned int, std::pair<vector<mData>, vector<mDatanp>>>& dict){
    //cout << "Merging index " << indexnum << endl;
	ifstream filez;
	ifstream filei;
	ofstream ofile;
    string pdir(NPDIR);
    Reader r;
    char flag = 'X';//determine the name of the output file
	filez.open(pdir + "X" + to_string(indexnum));
	filei.open(pdir + "L" + to_string(indexnum));

	ofile.open(pdir + "X" + to_string(indexnum + 1), ios::app | ios::binary);
	if(ofile.tellp() != 0){
        cout << "cannot merge to " << flag << indexnum + 1 << endl;
		ofile.close();
		ofile.open(pdir + "L" + to_string(indexnum + 1), ios::ate | ios::binary);
        flag = 'L';
	}

    cout << "Merging into " << flag << indexnum + 1 << "------------------------------------" << endl;

	long ostart;
	long oend;
	string file1 = "X" + to_string(indexnum);
	string file2 = "L" + to_string(indexnum);
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
        //cout << it1->termID << ' ' << it2->termID << endl;
		if( it1->termID == it2->termID ){
			//decode and merge
			//update meta data corresponding to the term
			vector<nPosting> vp1 = r.decompress_np(file1, it1->termID, dict);
            vector<nPosting> vp2 = r.decompress_np(file2, it2->termID, dict);
			vector<nPosting> vpout; //store the sorted result

			//use NextGQ to write the sorted vector of Posting to disk
			vector<nPosting>::iterator vpit1 = vp1.begin();
			vector<nPosting>::iterator vpit2 = vp2.begin();
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
            /*
            for( vector<Posting>::iterator it = vpout.begin(); it != vpout.end(); it ++){
                cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
            }
            */
			//update_t_meta(it1->termID, file1, dict);
            //update_t_meta(it2->termID, file2, dict);
			compress_np(vpout, filemeta, dict, indexnum + 1, flag);
			it1 ++;
			it2 ++;
		}
		else if( it1->termID < it2->termID ){
			vector<nPosting> vp = r.decompress_np(file1, it1->termID, dict);
            //update_t_meta(it1->termID, file1, dict);
            compress_np(vp, filemeta, dict, indexnum + 1, flag);
			it1 ++;
		}
		else if( it1->termID > it2->termID ){
            vector<nPosting> vp = r.decompress_np(file2, it2->termID, dict);
            //update_t_meta(it2->termID, file2, dict);
            compress_np(vp, filemeta, dict, indexnum + 1, flag);
			it2 ++;
		}
	}

    //decompress from the old index and then compress to the new one to update metadata is time-consuming
    //need to find a more efficient way to update metadata while tranfering positngs
	while (it1 != v1.end() ){
        vector<nPosting> vp = r.decompress_np(file1, it1->termID, dict);
        compress_np(vp, filemeta, dict, indexnum + 1, flag);
        it1 ++;
	}
	while (it2 != v2.end() ){
        vector<nPosting> vp = r.decompress_np(file2, it2->termID, dict);
        compress_np(vp, filemeta, dict, indexnum + 1, flag);
        it2 ++;
	}

	filez.close();
	filei.close();
	ofile.close();
    string filename1 = pdir + "X" + to_string(indexnum);
    string filename2 = pdir + "L" + to_string(indexnum);

    for( vector<f_meta>::iterator it = filemeta[file1].begin(); it != filemeta[file1].end(); it ++){
        update_t_meta(it->termID, file1, dict);
    }

    for( vector<f_meta>::iterator it = filemeta[file2].begin(); it != filemeta[file2].end(); it ++){
        update_t_meta(it->termID, file2, dict);
    }

    update_f_meta(filemeta, file1, file2);

    //deleting two files
    if( remove( filename1.c_str() ) != 0 )
        cout << "Error deleting file" << endl;

    if( remove( filename2.c_str() ) != 0 )
        cout << "Error deleting file" << endl;
}
