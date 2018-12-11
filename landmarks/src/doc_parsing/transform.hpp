#pragma once

#include <vector>

#include "diff.hpp"
#include "landmark/landmarkarray.hpp"

struct EditEntry {
    enum EditOperation {
        insertion,
        deletion,
        shift
    };

    EditEntry(bool l, EditOperation e, unsigned int a, unsigned int p, int t)
        : isLandEdit(l), editop(e), landID(a), pos(p), termID(t) {}

    bool isLandEdit; //false for Posting

    EditOperation editop; // 0 = insert, 1 = delete, 2 = shift
    unsigned int landID;
    int pos; // offset for posting, location/amount for landmarks
    int termID; // undefined for landmarks, normally -1 for landmarks
};

std::vector<EditEntry> transformDiff(const std::vector<int>& olddoc, const std::vector<int>& newdoc,
    LandmarkArray& landarray, std::vector<DiffRange>& editscript);