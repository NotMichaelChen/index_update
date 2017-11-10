#ifndef META_H
#define META_H

#include <string>

struct mData{
	std::string filename;
	long start_pos;
	long posting_offset;
	long docID_end;
	long second_end;
	long postingCount_offset;
	long size_offset;
	long end_offset;
};

#endif
