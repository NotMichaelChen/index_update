#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <utility>
#include <vector>

#include "externalpostings.h"
#include "Matcher/translate.h"
#include "Structures/documentstore.h"

struct MatcherInfo {
    MatcherInfo(std::vector<ExternNPposting> n, std::vector<ExternPposting> p, std::vector<Matcher::Translation> t)
        : NPpostings(n), Ppostings(p), translations(t) {}
    
    std::vector<ExternNPposting> NPpostings;
    std::vector<ExternPposting> Ppostings;
    std::vector<Matcher::Translation> translations;
};

//Updates the index given a new page
void indexUpdate(std::string& url, std::string& newpage);
//Generates new postings and translations from the new page
MatcherInfo makePosts(Structures::DocumentTuple& olddoc, std::string& newpage);

#endif