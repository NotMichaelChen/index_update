#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cmath>
#include <algorithm>
#include <sstream>
#define NO_DOC 10
#define INDEX "./test_data/compressedIndex"
#define INFO "./test_data/docInfo"
#define MEMORY_LIMIT 4096
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
struct mData{
	//need number of blocks?
	int num_posting;//number of postings
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
	
	mData compress_p(std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_fragID, std::vector<unsigned int>& v_pos){
		ofstream ofile;//positional inverted index
		ofile.open("./disk_index/positional");

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

		ofile.close();
	}

	void compress_p(std::vector<Posting>& pList){
		//pass in forward index of same termID
		//compress positional index

		std::vector<unsigned int> v_docID;
		std::vector<unsigned int> v_fragID;
		std::vector<unsigned int> v_pos;
		mData mmData;
		unsigned int num_of_p = 0;
		bool finished = true;

		unsigned int currID = pList[0].termID;//the ID of the term that is currently processing
		for(std::vector<Posting>::iterator it = pList.begin(); it != pList.end(); it++){
			while(it->termID == currID){
				if(v_docID.size() < MEMORY_LIMIT){
					v_docID.push_back(it->docID);
					v_fragID.push_back(it->fragID);
					v_pos.push_back(it->pos);
					it ++;
					num_of_p ++;
				}
				else{
					finished = false;
					break;
				}
			}
			currID = it->termID;
			mmData = compress_p(v_docID, v_fragID, v_pos);
			if (finished){
				mmData.num_posting = num_of_p;
				num_of_p = 0;
			}
			finished = true;
			//To-Do: add mmdata to the dictionary of corresponding term
			v_docID.clear();
			v_fragID.clear();
			v_pos.clear();
			it --;
		}
	}


	std::vector<Posting> read_forward_index(){
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
				num = 0;
				p = 0;
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
			}

		}
		index.close();
		info.close();

		std::sort(invert_index.begin(), invert_index.end(), less_than_key());
		/*
		for(vector<Posting>::iterator it = invert_index.begin(); it != invert_index.end(); it ++){
			cout << it->termID << ' ' << it->docID << ' '<< it->pos << endl;
		}
		*/

		return invert_index;
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

	Posting NextGQ(){

	}
};

int main(){
	Compressor comp;
	vector<Posting> inverted_ind = comp.read_forward_index();
	comp.compress_p(inverted_ind);

	return 0;
}