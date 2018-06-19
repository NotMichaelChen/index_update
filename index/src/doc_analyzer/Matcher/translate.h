#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <vector>

#include "block.h"

//Represents a translation that can be applied to an old posting to find its new
//position
struct Translation {
    Translation(int l, int o, int n) : loc(l), oldlen(o), newlen(n) {}
    
    //The location of the edited block in the old document
    int loc;
    //The length of the edited block in the old document
    int oldlen;
    //The length of the edited block in the new document
    int newlen;
};

//Get a list of translations given a list of common blocks between two files
std::vector<Translation> getTranslations(int oldfilelen, int newfilelen, std::vector<Block> commonblocks);
//Applys a translation to a given index
//Returns a negative number if the index is invalid
int applyTranslation(int oldindex, Translation t);

#endif