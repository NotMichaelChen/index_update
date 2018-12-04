#include "posting.hpp"

bool Posting::operator<(const Posting& rhs) {
    return docID < rhs.docID;
}

bool nPosting::operator<(const nPosting& rhs) {
    return docID < rhs.docID;
}