#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <vector>
#include <string>
#include <fstream>
#include <map>
#include "meta.hpp"
#include "posting.hpp"
#include "strless.hpp"

class Compressor{
public:
	std::vector<std::string> read_directory( std::string path );

  	void update_f_meta(std::map<std::string, std::vector<f_meta>, strless>& filemeta, std::string s1, std::string s2);

	void update_t_meta(unsigned int termID, std::string file, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict);

	void copy_and_paste(std::ifstream& ifile, std::ofstream& ofile, long start, long end);

	void merge_p(std::map<std::string, std::vector<f_meta>, strless>& filemeta, int indexnum, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict);

	void merge_test(std::map<std::string, std::vector<f_meta>, strless>& filemeta, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict);

	void write(std::vector<uint8_t> num, std::ofstream& ofile);

    bool check_contain(std::vector<std::string> v, std::string f);

	std::vector<char> read_com(std::ifstream& infile);

	std::vector<uint8_t> VBEncode(unsigned int num);

	std::vector<uint8_t> VBEncode(std::vector<unsigned int>& nums);

	std::vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv, std::vector<unsigned int> &last_id_biv);

	std::vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv);

	mData compress_p(std::string namebase, std::ofstream& ofile, f_meta& fm, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_fragID, std::vector<unsigned int>& v_pos);

	void compress_p(std::vector<Posting>& pList, std::map<std::string, std::vector<f_meta>, strless>& filemeta, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict, int indexnum = 0, char prefix = 'a');

	void start_compress(std::map<std::string, std::vector<f_meta>, strless>& filemeta, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict);

	std::vector<uint8_t> compress_freq(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv);

	mDatanp compress_np(std::string namebase, std::ofstream& ofile, f_meta& fm, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_freq, std::vector<unsigned int>& v_sign);

	void compress_np(std::vector<nPosting>& npList, std::map<std::string, std::vector<f_meta>, strless>& filemeta, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict, int indexnum = 0, char prefix = 'a');

	void merge_np(std::map<std::string, std::vector<f_meta>, strless>& filemeta, int indexnum, std::map<unsigned int, std::pair<std::vector<mData>, std::vector<mDatanp>>>& dict);
};

#endif
