#pragma once

struct Landmark {
    Landmark(unsigned int l, unsigned int p, int n, int pr) :
        landID(l), pos(p), nextLandInd(n), prevLandInd(pr)
    {}

    unsigned int landID;
    unsigned int pos;
    int nextLandInd;
    int prevLandInd;
};