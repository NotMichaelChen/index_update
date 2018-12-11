#include "landmarkdirectory.hpp"

LandmarkArray& LandmarkDirectory::getLandmarkArray(unsigned int docID) {
    return directory[docID];
}
