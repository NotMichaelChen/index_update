#include "update.hpp"

#include <vector>
#include <sstream>
#include <cstdlib>

#include "dtl/dtl.hpp"
#include "lexicon.hpp"

#include "stringencoder.h"

namespace Landmark {
	
	std::vector<EditTuple> diff(StringEncoder& docs, Lexicon& lex) {
		std::vector<int> olddoc, newdoc;
		std::stringstream ses;
		
		//Copy the int vectors from stringencoder
		std::copy(docs.getOldIter(), docs.getOldEnd(), olddoc.begin());
		std::copy(docs.getNewIter(), docs.getNewEnd(), newdoc.begin());
		
		//Get the SES
		dtl::Diff<int> editlist(olddoc, newdoc);
		editlist.compose();
		editlist.printSES(ses);
		
		//Edit transcript
		std::vector<EditTuple> editscript;
		
		std::string diff;
		int pos = 0;
		while(ses >> diff) {
			//Maybe we should crash here instead
			if(diff.length() == 0) {
				pos++;
				continue;
			}
			
			if(diff[0] == '+')
				editscript.emplace_back(Ins, pos, lex.getWord(docs.decodeNum(abs(std::stoi(diff)))));
			else if(diff[0] == '-')
				editscript.emplace_back(Del, pos, lex.getWord(docs.decodeNum(abs(std::stoi(diff)))));
				
			pos++;
		}
		
		return editscript;
	}
}
