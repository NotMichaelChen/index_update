#ifndef COMPARISON_H
#define COMPARISON_H

#include "posting.hpp"

bool operator< (const Posting& p1, const Posting& p2);

bool operator> (const Posting& p1, const Posting& p2);

bool operator== (const Posting& p1, const Posting& p2);

bool operator< (const nPosting& p1, const nPosting& p2);

bool operator> (const nPosting& p1, const nPosting& p2);

bool operator== (const nPosting& p1, const nPosting& p2);

struct less_than_key{
    inline bool operator() (const Posting& p1, const Posting& p2);
};

#endif
