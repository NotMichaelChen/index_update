#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <utility>
#include <vector>

#include "externalpostings.h"
#include "Matcher/translate.h"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

//A collection of positional/nonpositional postings and translation vector
struct MatcherInfo {
    MatcherInfo(std::vector<ExternNPposting> n, std::vector<ExternPposting> p, std::vector<Matcher::Translation> t, int m)
        : NPpostings(n), Ppostings(p), translations(t), maxfragID(m) {}
    
    std::vector<ExternNPposting> NPpostings;
    std::vector<ExternPposting> Ppostings;
    std::vector<Matcher::Translation> translations;
    int maxfragID;
};

//Updates the index given a new page
MatcherInfo indexUpdate(std::string& url, std::string& newpage, std::string& timestamp, Structures::DocumentStore& docstore, Structures::TranslationTable& transtable);
//Generates new postings and translations from the new page
MatcherInfo makePosts(Structures::DocumentTuple& olddoc, std::string& newpage);

#endif
