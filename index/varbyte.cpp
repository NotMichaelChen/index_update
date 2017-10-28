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
