#include "landmarkarray.hpp"

#include <stdexcept>

unsigned int LandmarkArray::getNextID() {
    return nextID;
}

void LandmarkArray::incrNextID() {
    nextID++;
}

unsigned int LandmarkArray::getAndIncrNextID() {
    unsigned int tmp = nextID;
    ++nextID;
    return tmp;
}

LandmarkArray::LmPointer LandmarkArray::getLandmark(unsigned int pos) {
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

LandmarkArray::LmPointer LandmarkArray::getLandmarkAfter(unsigned int pos) {
    auto landiter = landmarks.begin();
    while(landiter != landmarks.end()) {
        if(landiter->pos >= pos) {
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

LandmarkArray::LmPointer LandmarkArray::getNextLandmark(LmPointer curiter) {
    int index = curiter->nextLandInd;
    if(index <= 0)
        return landmarks.end();
    return landmarks.begin() + index;
}

LandmarkArray::LmPointer LandmarkArray::getPreviousLandmark(LmPointer curiter) {
    int index = curiter->prevLandInd;
    if(index <= 0)
        return curiter;
    return landmarks.begin() + index;
}

std::vector<LandmarkArray::LmPointer>
LandmarkArray::getLandmarkRange(unsigned int start, unsigned int end) {
    LmPointer landiter = getLandmark(start);

    std::vector<LmPointer> landmarkiters;

    // Get all landmarks that fall in range
    while(landiter != landmarks.end() && landiter->pos <= end) {
        landmarkiters.push_back(landiter);
        landiter = this->getNextLandmark(landiter);
    }

    return landmarkiters;
}

void LandmarkArray::refreshLandmark(LmPointer landiter) {
    landiter->landID = nextID;
    nextID++;
}

void LandmarkArray::shiftLandmarks(LmPointer landiter, int amount) {
    while(landiter != landmarks.end()) {
        landiter->pos += amount;
        landiter = getNextLandmark(landiter);
    }
}

void LandmarkArray::deleteLandmarks(std::vector<LmPointer>& landitervec) {

    for(LmPointer landiter : landitervec) {
        if(landitervec.empty())
            throw std::runtime_error("Error: trying to delete from empty landmark array");

        size_t landindex = landiter - landmarks.begin();
        // Update swapped element's prev and next index references
        // do this before actually swapping
        auto itertoswap = std::prev(landmarks.end());

        auto before = getPreviousLandmark(itertoswap);
        if(before != landmarks.end())
            before->nextLandInd = landindex;

        auto after = getNextLandmark(itertoswap);
        if(after != landmarks.end())
            after->prevLandInd = landindex;
        
        // Update deleted element's prev and next index references
        auto beforetodelete = getPreviousLandmark(landiter);
        auto aftertodelete = getNextLandmark(landiter);
        if(beforetodelete != landmarks.end() && aftertodelete != landmarks.end()) {
            beforetodelete->nextLandInd = aftertodelete - landmarks.begin();
            aftertodelete->prevLandInd = beforetodelete - landmarks.begin();
        }
        else if(beforetodelete != landmarks.end()) {
            beforetodelete->nextLandInd = -1;
        }
        else if(aftertodelete != landmarks.end()){
            aftertodelete->prevLandInd = -1;
        }

        // Swap with last element
        std::swap(landmarks[landindex], landmarks.back());
        // Pop last element
        landmarks.pop_back();
    }
}

LandmarkArray::LmPointer LandmarkArray::getEnd() {
    return landmarks.end();
}