#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <fstream>
#include "meta.hpp"

void write(vector<uint8_t> num, ofstream& ofile);

mData compress_p(string namebase, ofstream& ofile,
    std::vector<unsigned int>& v_docID,
    std::vector<unsigned int>& v_fragID,
    std::vector<unsigned int>& v_pos);

mDatanp compress_np(string namebase, ofstream& ofile,
	std::vector<unsigned int>& v_docID,
	std::vector<unsigned int>& v_freq,
	std::vector<unsigned int>& v_sign);

vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort,
    vector<uint8_t> &meta_data_biv,
    vector<unsigned int> &last_id_biv);

vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv);

vector<uint8_t> compress_freq(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv);

std::vector<Posting> decompress_p(string namebase, unsigned int termID)

std::vector<nPosting> decompress_np(string namebase, unsigned int termID)

#endif
