#include "lexicon.hpp"

#include <map>

namespace Landmark {
	Lexicon::Lexicon() : nextID(0) {}
	
	int Lexicon::getWord(std::string word) {
		if(lex.find(word) == lex.end()) {
			lex[word] = nextID;
			nextID++;
		}
		
		return lex[word];
	}
}
