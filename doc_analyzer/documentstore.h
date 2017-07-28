#ifndef DOCUMENTSTORE_H
#define DOCUMENTSTORE_H

#include <unordered_map>

class DocumentStore {
public:
    DocumentStore();
    std::pair<int, std::string> getDocument(std::string url);
    void insertDocument(std::string url, std::string doc);
    int getNextDocID();
private:
    //TEMPORARY DATA STRUCTURE
    std::unordered_map<std::string, std::pair<int, std::string>> store;
    int nextid;
};

#endif