#include "transform.hpp"

#include <stdexcept>

// Follows the split-block policy (all edit operations splits a block)
// TODO: Actually perform the operations so we have access to the latest landmarks
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
            //Invalidate old landmark
            transformedscript.emplace_back(true, EditEntry::deletion, landiter->landID, 0, -1);

            //Insert a new landmark
            //add one since entry.start describes inserting before that position
            unsigned int newlandmarkid = nextID;
            transformedscript.emplace_back(true, EditEntry::insertion, newlandmarkid, entry.start-1, -1);
            ++nextID;

            // Shift every landmark entry.len to the right
            // We always shift every landmark after the current one
            auto nextlanditer = landarray.getNextLandmark(landiter);
            if(nextlanditer != landarray.getEnd()) {
                transformedscript.emplace_back(true, EditEntry::shift, nextlanditer->landID, entry.len, -1);
            }

            // Issue new postings starting with new words then old deleted words
            unsigned int posend = (nextlanditer == landarray.getEnd()) ? olddoc.size() : nextlanditer->pos;
            for(int i : entry.terms) {
                transformedscript.emplace_back(false, EditEntry::insertion, landiter->landID, entry.start, i);
            }
            for(unsigned int i = entry.start; i < entry.start + entry.len; i++) {
                transformedscript.emplace_back(false, EditEntry::insertion, landiter->landID, entry.start, olddoc[i]);
            }
        }
        else {
            // Invalidate all landmarks that are part of the deletion range
            auto overlappedlandmarks = landarray.getLandmarkRange(entry.start, entry.start + entry.len);
            if(overlappedlandmarks.empty())
                throw std::runtime_error("Error: landmark range in deletion is empty");

            for(auto iter : overlappedlandmarks) {
                transformedscript.emplace_back(true, EditEntry::deletion, iter->landID, 0, -1);
            }

            // Shift all landmarks past the deletion range
            auto nextlanditer = landarray.getLandmarkAfter(entry.start + entry.len);
            if(nextlanditer != landarray.getEnd()) {
                // Only need to add one entry since it is implied that the other landmarks will be shifted
                transformedscript.emplace_back(true, EditEntry::shift, nextlanditer->landID, -entry.len, -1);
            }

            // Reinsert landmark at position of first landmark that was invalidated
            auto firstinvalidlandmark = overlappedlandmarks[0]; // Guaranteed to exist, otherwise we'd throw exception
            transformedscript.emplace_back(true, EditEntry::insertion, nextID, firstinvalidlandmark->pos, -1);
            unsigned int newlandmarkid = nextID;
            ++nextID;

            // Reinsert terms before deletion range then after deletion range until next landmark
            for(unsigned int i = firstinvalidlandmark->pos; i < entry.start; i++) {
                transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, i - firstinvalidlandmark->pos, olddoc[i]);
            }

            // represents the landmark after the end of the deletion block
            unsigned int posend = (nextlanditer == landarray.getEnd()) ? olddoc.size() : nextlanditer->pos;

            for(unsigned int i = entry.start+entry.len; i < posend; i++) {
                transformedscript.emplace_back(false, EditEntry::insertion, newlandmarkid, firstinvalidlandmark->pos, olddoc[i]);
            }
        }
    }

    return transformedscript;
}