#include "documentstore.h"

using namespace std;

DocumentStore::DocumentStore() : nextid(0) {}

pair<int, string> DocumentStore::getDocument(string url) {
    auto iter = store.find(url);
    if(iter == store.end())
        return make_pair(-1, "");
    else
        return iter->second;
}

void DocumentStore::insertDocument(std::string url, std::string doc) {
    store.insert({url, make_pair(nextid, doc)});
    ++nextid;
}

int DocumentStore::getNextDocID() {
    return nextid;
}