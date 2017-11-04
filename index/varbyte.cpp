#include <bitset>
#include "varbyte.hpp"

std::vector<uint8_t> VBEncode(unsigned int num){
	/**
	 * Variable byte encoding.
	 *
	 * @param: an umsigned interger to be compressed
	 * @return: a vector of bytes
	 */
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
	/**
	 * Encode a vector of unsigned intergers.
	 */
	vector<uint8_t> biv;
	vector<uint8_t> result;
	for( vector<unsigned int>::iterator it = nums.begin(); it != nums.end(); it ++){
		biv = VBEncode(*it);
		result.insert(result.end(), biv.begin(), biv.end());
	}
	return result;
}

std::vector<unsigned int> VBDecode(ifstream& ifile, long start_pos, long end_pos){//ios::ate
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

std::vector<unsigned int> VBDecode(vector<char>& vec){
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
