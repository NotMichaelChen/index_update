#include "landmarkdirectory.hpp"

LandmarkArray& LandmarkDirectory::getLandmarkArray(unsigned int docID) {
    return directory[docID];
}

size_t LandmarkDirectory::totalSize() {
    size_t size = 0;
    for(auto iter : directory) {
        size += iter.second.size();
    }
    return size;
}
