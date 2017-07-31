#ifndef META_H
#define META_H

#include <string>
#include <vector>

struct fileinfo{//a file that contains a part (or whole) postinglist
	std::string filename;
	long start_pos;
	long end_pos;
};

struct f_meta{
	unsigned int termID;
	long start_pos;
	long end_pos;
};

struct mData{
	//need number of blocks?
	std::string term;
	int index_num;//in which static index is the postinglist stored
	int num_posting;//number of postings

	std::vector<fileinfo> file_info;//how a postinglist is stored in multiple files
	long meta_doc_start;
	long meta_frag_start;
	long meta_pos_start;
	long posting_start;
	long frag_start;
	long pos_start;
	//long ID_offset;
};

#endif
