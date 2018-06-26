#ifndef DOCUMENTSTORE_H
#define DOCUMENTSTORE_H

#include <cpp_redis/cpp_redis>

#ifdef _WIN32
#include <Winsock2.h>
#endif /* _WIN32 */

struct DocumentTuple {
    DocumentTuple(unsigned int id, std::string d, unsigned int f, std::string t)
        : docID(id), doc(d), maxfragID(f), timestamp(t) {}
    
    unsigned int docID;
    std::string doc;
    //Refers to the next available fragID
    unsigned int maxfragID;
    std::string timestamp;
};

class DocumentStore {
public:
    DocumentStore();

    DocumentTuple getDocument(std::string url);
    void insertDocument(std::string url, std::string doc, int termlength, unsigned int maxfragID, std::string timestamp);

    //Document Statistics
    size_t getDocumentCount();
    int getDocLength(unsigned int docID);
    double getAverageDocLength();
    int getNextDocID();

    void dump();
    void clear();
private:
    //db 0:
    //{nextid}
    //{avgdoclen}
    //{doccount}
    //{url}, {docID, doc, doclength, maxfragID, timestamp}
    //db 2:
    //{docID}, {url}
    cpp_redis::client client;
};

#endif
