#include "varbyte.hpp"

#include <bitset>
#include <math.h>

std::vector<uint8_t> VBEncode(unsigned int num){
	/**
	 * Variable byte encoding.
	 *
	 * @param: an umsigned interger to be compressed
	 * @return: a vector of bytes
	 */
	std::vector<uint8_t> result;
	uint8_t b;
	while(num >= 128){
		int a = num % 128;
		std::bitset<8> byte(a);
		byte.flip(7);
		num = (num - a) / 128;
		b = byte.to_ulong();
		result.push_back(b);
	}
	int a = num % 128;
	std::bitset<8> byte(a);
	b = byte.to_ulong();
	result.push_back(b);
	return result;
}

std::vector<uint8_t> VBEncode(std::vector<unsigned int>& nums){
	/**
	 * Encode a std::vector of unsigned intergers.
	 */
	std::vector<uint8_t> biv;
	std::vector<uint8_t> result;
	for( std::vector<unsigned int>::iterator it = nums.begin(); it != nums.end(); it ++){
		biv = VBEncode(*it);
		result.insert(result.end(), biv.begin(), biv.end());
	}
	return result;
}

//compr_ind is the vector returned from read_com
std::vector<unsigned int> VBDecode(std::ifstream& ifile, std::vector<char>& compr_ind, long start_pos, long end_pos){//ios::ate
	ifile.seekg(start_pos);
	char c;
	unsigned int num;
	int p;
	std::vector<unsigned int> result;
    if(end_pos == 0) end_pos = ifile.end;

	for(std::vector<char>::iterator it = compr_ind.begin(); it != compr_ind.end(); it++){
		c = *it;
		std::bitset<8> byte(c);
		num = 0;
		p = 0;
		while(byte[7] == 1){
			byte.flip(7);
			num += byte.to_ulong()*pow(128, p);
			p++;
			it ++;
			c = *it;
			byte = std::bitset<8>(c);
		}
		num += (byte.to_ulong())*pow(128, p);

		result.push_back(num);
	}
	return result;
}

std::vector<unsigned int> VBDecode(std::vector<char>& vec){
	unsigned int num;
	std::vector<unsigned int> result;
	char c;
	int p;
	for(std::vector<char>::iterator it = vec.begin(); it != vec.end(); it++){
		c = *it;
		std::bitset<8> byte(c);
		num = 0;
		p = 0;
		while(byte[7] == 1){
			byte.flip(7);
			num += byte.to_ulong()*pow(128, p);
			p++;
			it ++;
			c = *it;
			byte = std::bitset<8>(c);
		}
		num += (byte.to_ulong())*pow(128, p);

		result.push_back(num);
	}
	return result;
}

std::vector<unsigned int> VBDecode(char* buffer, int length){
	unsigned int num;
	std::vector<unsigned int> result;
	char c;
	int p;
	for(int i = 0; i < length; i ++ ){
		c = buffer[i];
		std::bitset<8> byte(c);
		num = 0;
		p = 0;
		while(byte[7] == 1){
			byte.flip(7);
			num += byte.to_ulong()*pow(128, p);
			p++;
			i ++;
			c = buffer[i];
			byte = std::bitset<8>(c);
		}
		num += (byte.to_ulong())*pow(128, p);

		result.push_back(num);
	}
	return result;
}
