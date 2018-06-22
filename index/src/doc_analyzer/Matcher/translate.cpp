#include "translate.h"

#include <algorithm>

using namespace std;

vector<Translation> getTranslations(int oldfilelen, int newfilelen, vector<std::shared_ptr<Block>> commonblocks) {
    vector<Translation> translist;
    if(commonblocks.size() == 0)
        return translist;
    
    //Represents the text shifts from all previous transformations
    int shift = 0;
    //Represents the position in the old file that we base our transformations on
    int currentloc = 0;
    
    //Likely not necessary, but a useful guarantee
    sort(commonblocks.begin(), commonblocks.end(), compareOld);
    for(std::shared_ptr<Block> b : commonblocks) {
        int oldlength = b->oldloc - currentloc;
        int newlength = b->newloc - (currentloc+shift);
        
        if(oldlength != 0 || newlength != 0) {
            Translation trans(
                currentloc + shift,
                oldlength,
                newlength
            );
            
            translist.push_back(trans);
            
            shift += newlength - oldlength;
        }
        
        //want to go 1 past the edge; do not subtract 1 from run_size
        currentloc = b->oldloc + b->run.size();
    }
    
    //Add the last edit region if a common block does not extend to the end
    if(currentloc != oldfilelen || currentloc + shift != newfilelen) {
        Translation trans(
            currentloc + shift,
            oldfilelen - currentloc,
            newfilelen - (currentloc+shift)
        );
        
        translist.push_back(trans);
    }
    
    return translist;
}

int applyTranslation(int oldindex, Translation t) {
    if(oldindex < t.loc)
        return oldindex;
    else if(oldindex >= t.loc + t.oldlen)
        return oldindex + (t.newlen - t.oldlen);
    else
        return -1;
}