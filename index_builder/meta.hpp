#ifndef META_H
#define META_H

#include <string>
#include <vector>

struct f_meta{
	unsigned int termID;
	long start_pos;
	long end_pos;
};

struct mData{
	//need number of blocks?
	std::string term;
	int num_posting;//number of postings
	//how a postinglist is stored in multiple files
	std::string filename;
	long start_pos;
	long end_pos;
	long meta_doc_start;
	long meta_frag_start;
	long meta_pos_start;
	long posting_start;
	long frag_start;
	long pos_start;
	//long ID_offset;
};

#endif