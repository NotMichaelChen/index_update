#ifndef INDEX_HPP
#define INDEX_HPP

#include <map>
#include <vector>

namespace Landmark {
	
	struct Posting {
		unsigned int docID;
		unsigned int landmarkID;
		unsigned int offset;
	};
	
	class Index {
	public:
	private:
		std::map<unsigned int, std::vector<Posting>> index;
	};
}

#endif
