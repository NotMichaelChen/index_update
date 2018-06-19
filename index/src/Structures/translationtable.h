#ifndef TRANSLATIONTABLE_H
#define TRANSLATIONTABLE_H

#include <vector>
#include <cpp_redis/cpp_redis>

#ifdef _WIN32
#include <Winsock2.h>
#endif /* _WIN32 */

#include "doc_analyzer/Matcher/translate.h"

class TranslationTable {
public:
    TranslationTable();
    int apply(int docID, size_t fragID, int position);
    void insert(std::vector<Translation>& trans, int docID);
    //If a document gets reindexed, throw away its translation list
    void erase(int docID);
    void dump();
    void clear();
    
private:
    //key: docID
    //val: translations
    cpp_redis::client client;
};

#endif
