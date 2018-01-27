#include "meta.hpp"

mData shift_metadata(mData metadata, long shift) {
    metadata.start_pos += shift;
    metadata.last_docID += shift;
    metadata.blocksizes += shift;
    metadata.postings_blocks += shift;
    metadata.end_offset += shift;

    return metadata;
}