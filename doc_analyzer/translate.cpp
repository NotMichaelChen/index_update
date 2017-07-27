#include "translate.h"

#include <vector>

#include "block.h"

using namespace std;

vector<Translation> getTranslations(int oldfilelen, int newfilelen, vector<Block*> commonblocks) {
    vector<Translation> translist;
    if(commonblocks.size() == 0)
        return translist;
    
    int oldlast = oldfilelen - 1;
    int newlast = newfilelen - 1;
    
    //Represents the text shifts from all previous transformations
    int shift = 0;
    //Represents the position in the old file that we base our transformations on
    int currentloc = 0;
    
    for(Block* b : commonblocks) {
        int oldlength = b->oldloc - currentloc;
        int newlength = b->newloc - (currentloc+shift);
        
        if(oldlength != 0 && newlength != 0) {
            Translation trans = {
                currentloc + shift,
                oldlength,
                newlength
            };
            
            translist.push_back(trans);
        }
        
        shift += newlength - oldlength;
        //want to go 1 past the edge; do not subtract 1 from run_size
        currentloc = b->oldloc + b->run.size();
    }
    
    //Add the last edit region if a common block does not extend to the end
    if(currentloc != oldlast && currentloc + shift != newlast) {
        Translation trans = {
            currentloc + shift,
            oldlast - currentloc,
            newlast - (currentloc+shift)
        };
        
        translist.push_back(trans);
    }
    
    return translist;
}

int applyTranslation(int oldindex, Translation t) {
    if(index < t.loc)
        return index;
    else if(index > t.loc + t.oldlen)
        return index + (t.newlen - t.oldlen);
    else
        return -1;
}