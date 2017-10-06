#ifndef TRANSLATIONTABLE_H
#define TRANSLATIONTABLE_H

#include <vector>
#include <unordered_map>
#include <cpp_redis/cpp_redis>

#ifdef _WIN32
#include <Winsock2.h>
#endif /* _WIN32 */

#include "Matcher/translate.h"

namespace Structures {
    class TranslationTable {
    public:
        TranslationTable();
        int apply(int docID, int fragID, int position);
        void insert(std::vector<Matcher::Translation>& trans, int docID);
        //If a document gets reindexed, throw away its translation list
        void erase(int docID);
        
    private:
        //key: docID
        //val: translations
        cpp_redis::redis_client client;
    };
}
#endif
