#include "landmarkdirectory.hpp"

void LandmarkDirectory::insertLandmarkArray(
    unsigned int docID,
    std::vector<Landmark>& landmarks)
{
    directory[docID].insert(
        directory[docID].end(),
        landmarks.begin(),
        landmarks.end()
    );
}