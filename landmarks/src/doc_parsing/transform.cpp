#include "transform.hpp"

// Follows the split-block policy (all edit operations splits a block)
std::vector<EditEntry> transformDiff(const std::vector<int>& olddoc, const std::vector<int>& newdoc,
    LandmarkArray& landarray, std::vector<DiffRange>& editscript)
{
    std::vector<EditEntry> transformedscript;
    // Make copy so we can edit it but keep original value
    // We don't need to change nextID here, we will do it when applying the edit script
    unsigned int nextID = landarray.getNextID();

    for(DiffRange entry : editscript) {
        auto landiter = landarray.getLandmark(entry.start);
        
        if(entry.isIns) {
            //Insert a new landmark if insertion doesn't exactly overlap a landmark
            //add one since entry.start describes inserting before that position
            unsigned int newlandmarkid = landiter->landID;
            if(entry.start != landiter->pos) {
                transformedscript.emplace_back(true, EditEntry::insertion, nextID, entry.start-1, -1);

                ++nextID;
            }

            // Shift every landmark entry.len to the right
            // We always shift every landmark after the current one
            auto nextlanditer = landarray.getNextLandmark(landiter);
            if(nextlanditer != landarray.getEnd()) {
                transformedscript.emplace_back(true, EditEntry::shift, nextlanditer->landID, entry.len, -1);
            }

            // Issue delete postings for words from entry.start to end of landmark
            unsigned int posend = (nextlanditer == landarray.getEnd()) ? olddoc.size() : nextlanditer->pos;

            for(unsigned int i = entry.start; i < posend; i++) {
                transformedscript.emplace_back(false, EditEntry::deletion, newlandmarkid, i - entry.start, olddoc[i]);
            }

            // Issue new postings starting with new words then old deleted words
            for(int i : entry.terms) {
                transformedscript.emplace_back(false, EditEntry::insertion, landiter->landID, entry.start, i);
            }
            for(unsigned int i = entry.start; i < entry.start + entry.len; i++) {
                transformedscript.emplace_back(false, EditEntry::insertion, landiter->landID, entry.start, olddoc[i]);
            }
            
        }
        else {
            // If there are landmarks in the deletion range, delete them
            auto overlappedlandmarks = landarray.getLandmarkRange(entry.start, entry.start + entry.len);
            for(auto iter : overlappedlandmarks) {
                transformedscript.emplace_back(true, EditEntry::deletion, iter->landID, 0, -1);
            }

            // TODO: do we ever need to insert landmarks when deleting? We should just move the last landmark
            // Insert new landmark before delete range if it doesn't overlap a landmark
            unsigned int newlandmarkid = landiter->landID;
            if(entry.start-1 != landiter->pos && entry.start != 0) {
                transformedscript.emplace_back(true, EditEntry::deletion, nextID, entry.start-1, -1);
                newlandmarkid = nextID;
                ++nextID;
            }

            auto nextlanditer = landarray.getLandmarkAfter(entry.start + entry.len);

            // Shift every landmark entry.len to the left
            if(nextlanditer != landarray.getEnd()) {
                transformedscript.emplace_back(true, EditEntry::shift, nextlanditer->landID, -entry.len, -1);
            }

            // Issue deletion postings starting from entry.start to next landmark after entry.start+entry.len inclusive
            // represents the landmark after the end of the deletion block
            unsigned int posend = (nextlanditer == landarray.getEnd()) ? olddoc.size() : nextlanditer->pos;

            for(unsigned int i = entry.start; i < posend; i++) {
                transformedscript.emplace_back(false, EditEntry::deletion, newlandmarkid, i - entry.start, olddoc[i]);
            }

            // Issue new postings starting from entry end to next landmark
            for(unsigned int i = entry.start + entry.len; i < posend; ++i) {
                transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, i - entry.start, olddoc[i]);
            }
        }
    }

    return transformedscript;
}