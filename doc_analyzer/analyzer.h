#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <utility>

//Updates the index given a new page
void indexUpdate(std::string& url, std::string& newpage);
//Generates new postings and translations from the new page
void makePosts(std::string& url, unsigned int doc_id, std::string& oldpage, std::string& newpage);

#endif