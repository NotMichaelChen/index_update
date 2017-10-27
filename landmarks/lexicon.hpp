#ifndef LEXICON_HPP
#define LEXICON_HPP

#include <unordered_map>

namespace Landmark {
	class Lexicon {
	public:
		Lexicon();
		int getWord(std::string word);
	private:
		std::unordered_map<std::string, unsigned int> lex;
		unsigned int nextID;
	};
}

#endif
