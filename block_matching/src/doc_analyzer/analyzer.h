#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <vector>

#include "externalpostings.h"
#include "Matcher/translate.h"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

//A collection of positional/nonpositional postings and translation vector
struct MatcherInfo {
    MatcherInfo(std::unordered_map<std::string, ExternNPposting> n, std::vector<ExternPposting> p, std::vector<Translation> t, StringEncoder s, int m)
        : NPpostings(n), Ppostings(p), translations(t), se(s), maxfragID(m) {}

    std::unordered_map<std::string, ExternNPposting> NPpostings;
    std::vector<ExternPposting> Ppostings;
    std::vector<Translation> translations;
    StringEncoder se;

    int maxfragID;
    unsigned int docID;
};

//Updates the index given a new page
MatcherInfo indexUpdate(std::string& url, std::string& newpage, std::string& timestamp, DocumentStore& docstore, TranslationTable& transtable);
//Generates new postings and translations from the new page
MatcherInfo makePosts(DocumentTuple& olddoc, std::string& newpage);

#endif
