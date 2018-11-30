#include "posting.hpp"

bool Posting::operator<(const Posting& rhs) const {
    return docID < rhs.docID;
}

bool nPosting::operator<(const nPosting& rhs) const {
    return docID < rhs.docID;
}