#pragma once

#include <vector>

#include "libs/sparsepp/spp.h"
#include "landmark/landmark.hpp"

class LandmarkDirectory {
public:
    void insertLandmarkArray(unsigned int docID, std::vector<Landmark>& landmarks);
private:
    spp::sparse_hash_map<unsigned int, std::vector<Landmark>> directory;
};