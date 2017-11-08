#ifndef META_H
#define META_H

#include <string>

struct mData{
	std::string filename;
	long start_pos;
	long posting_offset;
	long size_offset;
	long postingCount_offset;
	long end_offset;
};

#endif
