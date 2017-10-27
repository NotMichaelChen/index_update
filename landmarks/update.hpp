#ifndef UPDATE_HPP
#define UPDATE_HPP

#include <vector>

#include "stringencoder.h"
#include "directory.hpp"
#include "lexicon.hpp"

namespace Landmark {
	
	enum Command {
		//EditTuple
		Ins,
		Del,
		//TransEditTuple directory
		Ins_L,
		Del_L,
		Shift_L,
		//TransEditTuple index
		Ins_P,
		Del_P,
	};
	
	struct EditTuple {
		EditTuple(Command c, unsigned int p, int w) : com(c), position(p), wordID(w) {}
		
		Command com;
		unsigned int position;
		//From the given stringencoder
		int wordID;
	};
	
	struct TransEditTuple {
		TransEditTuple(Command c, unsigned int l, int a, int w) : com(c), landmarkID(l), amount(a), wordID(w) {}
		
		Command com;
		unsigned int landmarkID;
		//Can mean either shift amount or location
		int amount;
		//Only used if adding/removing word into index
		int wordID;
	};
	
	std::vector<EditTuple> diff(StringEncoder& docs, Lexicon& lex);
	std::vector<TransEditTuple> transform(const std::vector<EditTuple>& transcript, const StringEncoder& olddoc, const LandmarkDirectory& dir);
	//TODO: add index as argument or make into method
	void apply(const std::vector<TransEditTuple>& transcript, LandmarkDirectory& dir);
}

#endif
