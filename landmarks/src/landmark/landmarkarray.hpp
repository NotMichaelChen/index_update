#pragma once

#include <vector>

#include "landmark.hpp"

class LandmarkArray {
public:
    LandmarkArray() : nextID(0) {};

    void insertLandmark();

    unsigned int getNextID();
    unsigned int getAndIncrNextID();
    void incrNextID();
    
    //TODO: typedef this
    // Gets the landmark associated with the given position
    // Both versions are inclusive
    std::vector<Landmark>::iterator getLandmark(unsigned int pos);
    std::vector<Landmark>::iterator getLandmarkAfter(unsigned int pos);

    std::vector<Landmark>::iterator getNextLandmark(std::vector<Landmark>::iterator curiter);
    std::vector<Landmark>::iterator getPreviousLandmark(std::vector<Landmark>::iterator curiter);

    // Gets all landmarks that fall between start and end
    // Any landmark within this range will be returned
    std::vector<std::vector<Landmark>::iterator> getLandmarkRange(
        unsigned int start,
        unsigned int end
    );

    std::vector<Landmark>::iterator getEnd();

private:
    std::vector<Landmark> landmarks;
    unsigned int nextID;
};