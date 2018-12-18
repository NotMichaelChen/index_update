#pragma once

#include <vector>

#include "landmark.hpp"

class LandmarkArray {
public:
    using LmPointer = std::vector<Landmark>::iterator;

    LandmarkArray() : nextID(0) {};

    unsigned int getNextID();
    unsigned int getAndIncrNextID();
    void incrNextID();
    
    // Gets the landmark associated with the given position
    // Both versions are inclusive
    LmPointer getLandmark(unsigned int pos);
    LmPointer getLandmarkAfter(unsigned int pos);

    LmPointer getNextLandmark(LmPointer curiter);
    LmPointer getPreviousLandmark(LmPointer curiter);

    // Gets all landmarks that fall between start and end
    // Any landmark within this range will be returned
    std::vector<LmPointer> getLandmarkRange(
        unsigned int start,
        unsigned int end
    );

    // Returns the ID of the new landmark
    unsigned int insertLandmark(unsigned int position);
    // Updates the given landmark to get a new landmark. Increments nextID
    void refreshLandmark(LmPointer landiter);
    void shiftLandmarks(LmPointer landiter, int amount);
    // All pointers in vec will be invalid after deletion
    void deleteLandmarks(std::vector<LmPointer>& landitervec);

    LmPointer getEnd();

private:
    std::vector<Landmark> landmarks;
    unsigned int nextID;
};