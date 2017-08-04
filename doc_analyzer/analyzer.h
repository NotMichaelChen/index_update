#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <utility>
#include <vector>

#include "postings.h"
#include "Matcher/translate.h"
#include "Structures/documentstore.h"

struct MatcherInfo {
    MatcherInfo(std::vector<NonPositionalPosting> n, std::vector<PositionalPosting> p, std::vector<Matcher::Translation> t)
        : NPpostings(n), Ppostings(p), translations(t) {}
    
    std::vector<NonPositionalPosting> NPpostings;
    std::vector<PositionalPosting> Ppostings;
    std::vector<Matcher::Translation> translations;
};

//Updates the index given a new page
void indexUpdate(std::string& url, std::string& newpage);
//Generates new postings and translations from the new page
MatcherInfo makePosts(Structures::DocumentTuple& olddoc, std::string& newpage);

#endif