#include "landmarkarray.hpp"

unsigned int LandmarkArray::getNextID() {
    return nextID;
}

std::vector<Landmark>::iterator LandmarkArray::getLandmark(unsigned int pos) {
    auto landiter = landmarks.begin();
    while(landiter != landmarks.end()) {
        //TODO: Refactor logic
        //We are at the right landmark if its position is less than the diff position
        //and the next landmark is greater than the diff position, but only if there's
        //a next landmark.
        if(landiter->pos <= pos) {
            if(landiter->nextLandInd != -1) {
                if(landmarks[landiter->nextLandInd].pos > pos) {
                    break;
                }
            }
            else {
                break;
            }
        }
        landiter++;
    }

    return landiter;
}

std::vector<Landmark>::iterator LandmarkArray::getLandmarkAfter(unsigned int pos) {
    auto landiter = landmarks.begin();
    while(landiter != landmarks.end()) {
        if(landiter->pos > pos) {
            if(landiter->prevLandInd != -1) {
                if(landmarks[landiter->prevLandInd].pos < pos) {
                    break;
                }
            }
            else
                break;
        }
        landiter++;
    }

    return landiter;
}

std::vector<Landmark>::iterator LandmarkArray::getNextLandmark(std::vector<Landmark>::iterator curiter) {
    int index = curiter->nextLandInd;
    if(index <= 0)
        return landmarks.end();
    return landmarks.begin() + index;
}


std::vector<std::vector<Landmark>::iterator>
LandmarkArray::getLandmarkRange(unsigned int start, unsigned int end) {
    //TODO: make not inclusive. We don't want a landmark if it lands exactly on a boundary
    // If startiter isn't specified, get it ourselves
    std::vector<Landmark>::iterator landiter = getLandmark(start);

    std::vector<std::vector<Landmark>::iterator> landmarkiters;

    // Get all landmarks that fall in range
    while(landiter != landmarks.end() && landiter->pos < end) {
        landmarkiters.push_back(landiter);
        landiter = this->getNextLandmark(landiter);
    }

    // Check if the last landmark is in range
    if(this->getNextLandmark(landmarkiters.back())->pos )
        landmarkiters.pop_back();

    return landmarkiters;
}

std::vector<Landmark>::iterator LandmarkArray::getEnd() {
    return landmarks.end();
}