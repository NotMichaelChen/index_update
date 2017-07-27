#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>

#define NO_DOC 10 //temporary use
#define POSTING_LIMIT 1000 //make sure doesn't exceed memory limit
#define INDEX "./test_data/compressedIndex"
#define INFO "./test_data/docInfo"
#define PDIR "./disk_index/positional/"

using namespace std;

class Posting{
public:
	Posting(unsigned int id, unsigned int d, unsigned int f = 0, unsigned int p = 0){
		termID = id;
		docID = d;
		fragID = f;
		pos = p;
	}

	unsigned int termID;
	unsigned int docID;
	unsigned int fragID;
	unsigned int pos;
};
/*
class PostingList{
public:
	PostingList(unsigned int id, unsigned int d, unsigned int f = 0, unsigned int p = 0){
		Posting p(id, d, f, p);
		pl.push_back(p);
	}

	void add(Posting p){
		pl.push_back(p);
	}

	vector<Posting> pl;
};
*/

struct fileinfo{//a file that contains a part (or whole) postinglist
	string filename;
	long start_pos;
	long end_pos;
};

struct mData{
	//need number of blocks?
	int index_num;//in which static index is the postinglist stored
	int num_posting;//number of postings

	std::vector<fileinfo> file_info;//how a postinglist is stored in multiple files
	long start_pos;
	long meta_doc_start;
	long meta_frag_start;
	long meta_pos_start;
	long posting_start;
	long frag_start;
	long pos_start;
	//long ID_offset;
};

struct less_than_key
{
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

class Compressor{
public:
	std::vector<std::string> read_directory( std::string& path ){
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
      			result.push_back( std::string( de->d_name ) );
      		}
			closedir( dp );
    	}
  		return result;
  	}

  	void update_meta(string s1, string s2){

  	}

	void merge(indexnum){
		ifstream filez;
		ifstream fileI;
		ofstream ofile;
		filez.open(PDIR + "Z" + to_string(indexnum));
		filei.open(PDIR + "I" + to_string(indexnum));

		ofile.open(PDIR + "Z" + to_string(indexnum + 1), ios::ate | ios::binary);
		if(ofile.tellg() != 0){
			ofile.close();
			ofile.open(PDIR + "I0", ios::ate | ios::binary);
		}

		update_meta(PDIR + "Z" + to_string(indexnum), PDIR + "I" + to_string(indexnum));

		vector<Posting> indexz = Reader::read(filez);
		vector<Posting> indexi = Reader::read(filei);
		//assume postinglist can be loaded into memory entirely

		
	}

	void merge_test(PDIR){
		int indexnum = 0;
		vector<string> files = read_directory(PDIR);

		while(!none_of(begin(files), std::end(files), "I" + to_string(indexnum))){
			//if In exists already, merge In with Zn
			merge(indexnum);

			indexnum ++;
		}
	}

	void write(vector<uint8_t> num, ofstream& ofile){
		for(vector<uint8_t>::iterator it = num.begin(); it != num.end(); it++){
			ofile.write(reinterpret_cast<const char*>(&(*it)), 1);
		}
	}

	std::vector<char> read_com(ifstream& infile){
		char c;
		vector<char> result;
		while(infile.get(c)){
			result.push_back(c);
		}
		return result;
	}

	std::vector<uint8_t> VBEncode(unsigned int num){
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

	std::vector<uint8_t> VBEncode(vector<unsigned int>& nums){
		vector<uint8_t> biv;
		vector<uint8_t> result;
		for( vector<unsigned int>::iterator it = nums.begin(); it != nums.end(); it ++){
			biv = VBEncode(*it);
			result.insert(result.end(), biv.begin(), biv.end());
		}
		return result;
	}

	vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv, vector<unsigned int> &last_id_biv){
		if(method){
			std::vector<unsigned int> block;
			std::vector<unsigned int>::iterator it = field.begin();
			std::vector<uint8_t> field_biv;
			std::vector<uint8_t> biv;

			unsigned int prev = *it;
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
				last_id_biv.push_back(prev);//the first element of every block needs to be stored
				field_biv.insert(field_biv.end(), biv.begin(), biv.end());
				meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
			}
			return field_biv;
		}
	}

	vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv){
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
	
	mData compress_p(ofstream& ofile, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_fragID, std::vector<unsigned int>& v_pos){
		
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
		fileinfo fi;
		fi.start_pos = ofile.tellp();
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

		fi.end_pos = ofile.tellp();
		meta.file_info.push_back(fi);//store the start and end position of postinglist in this file
	}

	void compress_p(std::vector<Posting>& pList, std::map<string, vector<int>>& filemeta){
		//pass in forward index of same termID
		//compress positional index
		ofstream ofile;//positional inverted index
		string filename = PDIR + "Z0";
		ofile.open(filename, ios::ate | ios::binary);
		if(ofile.tellg() != 0){
			ofile.close();
			fielname = PDIR + "I0";
		}
		ofile.open(filename, ios::ate | ios::binary);
		vector<int> termIDs;

		std::vector<unsigned int> v_docID;
		std::vector<unsigned int> v_fragID;
		std::vector<unsigned int> v_pos;
		mData mmData;
		unsigned int num_of_p = 0;//number of posting of a certain term

		unsigned int currID = pList[0].termID;//the ID of the term that is currently processing
		for(std::vector<Posting>::iterator it = pList.begin(); it != pList.end(); it++){
			while(it->termID == currID){
				v_docID.push_back(it->docID);
				v_fragID.push_back(it->fragID);
				v_pos.push_back(it->pos);
				it ++;
				num_of_p ++;
			}
			termIDs.push_back(currID);
			currID = it->termID;
			mmData = compress_p(ofile, v_docID, v_fragID, v_pos);
			mmData.num_posting = num_of_p;
			
			mmData.file_info.filename = filename;
			//To-Do: add mmdata to the dictionary of corresponding term

			num_of_p = 0;
			v_docID.clear();
			v_fragID.clear();
			v_pos.clear();
			it --;//before exit while loop, iterator is added but the corresponding value is not pushed to vector
		}

		filemeta[filename] = termIDs;
		ofile.close();
		merge_test();//see if need to merge
	}

	void start_compress(map<string, vector<int>>& filemeta){
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
				if (invert_index.size() > POSTING_LIMIT){ // make sure doesn't exceed memory
					std::sort(invert_index.begin(), invert_index.end(), less_than_key());
					compress_p(invert_index, filemeta);
					invert_index.clear()
				}
			}

		}
		index.close();
		info.close();
	}
};

class Reader{
public:

	std::vector<char> read_com(ifstream& infile){
		char c;
		vector<char> result;
		while(infile.get(c)){
			result.push_back(c);
		}
		return result;
	}

	std::vector<int> VBDecode(string filename){
		ifstream ifile;
		ifile.open(filename, ios::binary);
		char c;
		int num;
		int p;
		vector<int> result;
		vector<char> vec = read_com(ifile);

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

	std::vector<Posting> read(&ifstream file){
		
	}

	Posting NextGQ(){

	}
};

int main(){
	Compressor comp;
	map<string, vector<int>> filemeta;
	comp.start_compress(filemeta;);
	

	return 0;
}