#include <iostream>
#include <fstream>
using namespace std;

class Posting{
public:
	Posting(unsigned int id, unsigned int d, unsigned int f = 0, unsigned int p = 0){
		docID = d;
		fragID = f;
		pos = p;
	}

private:
	unsigned int termID;
	unsigned int docID;
	unsigned int fragID;
	unsigned int pos;
};

class PostingList{
public:
	PostingList(int id){
		termID = id;
	}

	void add(Posting p){
		pl.push_back(p);
	}

private:
	int termID;
	vector<Posting> pl;
};

class Compressor{
public:
	void writetoDisk(){

	}

	vector<int> VBEncode(int num){
		vector<int> result;
		
		while ((num) > 127) {
			result.push_back(1);
			std::vector<int>::iterator it = result.end();
			result.insert(it, 7, 1);
			num -= 127;
		}
		mybits += '1';
		mybits += std::bitset<7>(*it).to_string();
		buffer += mybits;
	return buffer;
	}

	vector<int> encode(vector<unsigned int> uncomp){
		vector<int> biv;
		vector<int> result;
		for (vector<unsigned int>::iterator it = uncomp.begin(); it != uncomp.end(); ++it) {
			biv = Compressor::VBEncode(*it);
			result.insert(result.end(), biv.start(), biv.end());
			biv.clear();
		}
		return result;
	}

	vector<int> compress(std::vector<unsigned int> field; int method; int sort; vector<unsigned int>& last_element = {}){
		if(method){
			std::vector<unsigned int> block;
			std::vector<unsigned int>::iterator it = field.begin();
			std::vector<int> field_biv;
			std::vector<int> biv;

			if(sort){
				int prev = 0;
				while(it != field.end()){
					int size_block = 0;
					block.clear();

					while(size_block < 64 && it != field.end()){
						block.push_back(*it - prev);
						prev = *it;
						size_block ++;
						it ++;
					}
					biv = encode(field);
					last_element.push_back(prev);
					field_biv.insert(field_biv.end(), biv.begin(), biv.end());
				}

				return field_biv;

			}else{

			}
		}
	}
	

	mData compress(std::vector<Index::Posting> index; ofstream& diskIndex){
		std::vector<unsigned int> v_docID;
		std::vector<unsigned int> v_fragID;
		std::vector<unsigned int> v_pos;
		std::vector<unsigned int> v_last_id;
		std::vector<int> last_id_biv;
		std::vector<int> biv;

		for(std::vector<Index::Posting>::iterator it = index.begin(); it != index.end(); it++){
			v_docID.push_back(it->docID);
			v_fragID.push_back(it->fragID);
			v_pos.push_back(it->pos);
		}

		biv = compress(v_docID, 1, 1, v_last_id);
		last_id_biv = encode(v_last_id);


		if(v_fragID.size() != 0){
			compress(v_fragID, 1, 0);
		}
		if(v_pos.size() != 0){
			compress(v_pos, 1, 0);
		}

		
	}

private:
	struct mData{
		int num_block;
		int num_posting;
		long posting_start;
		long ID_offset;
	};
};

class Reader{
public:
	int VBDecode(){

	}

	NextGQ(){

	}
};