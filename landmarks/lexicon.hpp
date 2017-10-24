#ifndef LEXICON_HPP
#define LEXICON_HPP

#include <unordered_map>

namespace Landmark {
	class Lexicon {
	public:
	private:
		std::unordered_map<unsigned int, std::string> lex;
		unsigned int nextID;
	};
}

#endif
