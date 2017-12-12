#include "meta.hpp"

mData shift_metadata(mData metadata, long shift) {
    metadata.start_pos += shift;
    metadata.posting_offset += shift;
    metadata.docID_end += shift;
    metadata.second_end += shift;
    metadata.postingCount_offset += shift;
    metadata.size_offset += shift;
    metadata.end_offset += shift;

    return metadata;
}