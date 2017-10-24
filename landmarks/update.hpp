#ifndef UPDATE_HPP
#define UPDATE_HPP

#include <vector>

#include "stringencoder.h"
#include "directory.hpp"

namespace Landmark {
	
	enum Command {
		//EditTuple
		Ins,
		Del,
		Repl,
		//TransEditTuple directory
		Ins-L,
		Del-L,
		Shift-L,
		//TransEditTuple index
		Ins-P,
		Del-P,
		Repl-P
	};
	
	struct EditTuple {
		Command com;
		unsigned int position;
		//From the given stringencoder
		int wordID;
	};
	
	struct TransEditTuple {
		Command com;
		unsigned int landmarkID;
		int amount;
		//Only used if adding/removing word into index
		int wordID;
	};
	
	std::vector<EditTuple> diff(const StringEncoder& docs);
	std::vector<TransEditTuple> transform(const std::vector<EditTuple>& transcript, const StringEncoder& olddoc, const LandmarkDirectory& dir);
	//TODO: add index as argument or make into method
	void apply(const std::vector<TransEditTuple>& transcript, LandmarkDirectory& dir);
}

#endif
