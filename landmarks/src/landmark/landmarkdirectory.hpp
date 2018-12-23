#pragma once

#include <vector>

#include "libs/sparsepp/spp.h"
#include "landmark/landmark.hpp"
#include "landmark/landmarkarray.hpp"

class LandmarkDirectory {
public:

    LandmarkArray& getLandmarkArray(unsigned int docID);

    size_t totalSize();

private:
    spp::sparse_hash_map<unsigned int, LandmarkArray> directory;
};