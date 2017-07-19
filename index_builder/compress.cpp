#include <iostream>
#include <fstream>
#include "index.hpp"
using namespace std;

class Compressor{
public:
	void writetoDisk(){

	}

	vector<int> VBEncode(int num){

	}

	vector<int> encode(vector<int>){}


	mData compress(Index::postingList pl; ofstream& diskIndex; int method = 1){
		std::vector<int> binaryVec;//compressed integer
		std::vector<int> ID_block;//uncompressed IDs
		std::victor<int> freq_block;//uncompressed freqs
		std::vector<int> ID_biv;//compressed IDs
		std::victor<int> freq_biv;//compressed freqs
		std::vector<int> last_id_biv;//metadata doc ID
		std::vector<int> doc_size_biv;
		std::vector<int> freq_size_biv;

		std::vector<unsigned int>::iterator = itID;
		std::vector<unsigned int>::iterator = itfreq;
		int doc_size = 0;
		int frep_size = 0;

		while(itID != pl->postings.end()){
			int size_blocks = 0;
			ID_block.clear();
			freq_block.clear();

			while(size_blocks < 64 && itID != pl->postings.end()){
				ID_block.push_back(*itID);
				freq_block.push_back(*itfreq);
				size_blocks ++;
				itID++;
				itfreq++;
			}

			//compress ID and freq:
			binaryVec = Compressor::encode(ID_block);
			ID_biv.insert(ID_biv.end(), binaryVec.begin(), binaryVec.end());
			binaryVec = Compressor::encode(freq_block);
			freq_biv.insert(freq_biv.end(), binaryVec.begin(), binaryVec.end());

			//compress metadata:
			binaryVec = Compressor::VBEncode(*it);
			last_id_biv.insert(last_id_biv.end(), binaryVec.begin(), binaryVec.end());
			doc_size = ID_biv.size() - doc_size;
			freq_size = freq_biv.size() - freq_size;
			binaryVec = Compressor::VBEncode(doc_size);
			doc_size_biv.insert(doc_size_biv.end(), binaryVec.begin(), binaryVec.end());
			binaryVec = Compressor::VBEncode(freq_size);
			freq_size_biv.insert(freq_size_biv.end(), binaryVec.begin(), binaryVec.end());
		}
		Compressor::mData.posting_start = diskIndex.tellp();
		Compressor::writetoDisk(diskIndex, last_id_biv);
		Compressor::writetoDisk(diskIndex, doc_size_biv);
		Compressor::writetoDisk(diskIndex, freq_size_biv);
		Compressor::mData.ID_offset = diskIndex.tellp();
		Compressor::writetoDisk(diskIndex, ID_biv);
		Compressor::mData.freq_offset = diskIndex.tellp();
		Compressor::writetoDisk(diskIndex, freq_biv);

		
	}

private:
	struct mData{
		int num_block;
		int num_posting;
		long posting_start;
		long ID_offset;
		long freq_offset;
	};
};

class Reader{
public:
	int VBDecode(){

	}

	NextGQ(){

	}
};