#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <unordered_map>
#include <vector>

namespace Landmark {
	
	struct LandmarkPos {
		unsigned int landmarkID;
		unsigned int pos;
	};
	
	class LandmarkDirectory {
	public:
		void insertLandmark(unsigned int docID, LandmarkPos ldmrk);
	private:
		std::unordered_map<unsigned int, std::vector<LandmarkPos>> dir;
	};
}

#endif
