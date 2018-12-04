#include "documentstore.hpp"

#include <sys/socket.h>
#include <sys/time.h>

#ifdef _WIN32
#include <Winsock2.h>
#endif /* _WIN32 */

using namespace std;

//https://math.stackexchange.com/a/1567342

//collectionsize refers to the size of the collection before removal/adding
double updateAverageRemove(double avg, unsigned int toremove, unsigned int collectionsize) {
    if(collectionsize == 1)
        return 0;
    else
        return ((avg * collectionsize) - toremove) / (collectionsize - 1);
}

double updateAverageAdd(double avg, unsigned int toadd, unsigned int collectionsize) {
    return avg + (toadd - avg) / (collectionsize+1);
}

DocumentStore::DocumentStore() {
    #ifdef _WIN32
        //! Windows netword DLL init
        WORD version = MAKEWORD(2, 2);
        WSADATA data;
        
        if (WSAStartup(version, &data) != 0) {
            std::cerr << "WSAStartup() failure" << std::endl;
            return -1;
        }
    #endif /* _WIN32 */
    
    //cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);
    
    client.connect("127.0.0.1", 6379);
    client.select(0);
    
    client.setnx("nextid", "0");
    client.setnx("avgdoclen", "0");
    client.setnx("doccount", "0");
    client.sync_commit();
}

DocumentTuple DocumentStore::getDocument(string url) {
    vector<cpp_redis::reply> response;
    
    client.lrange(url, 0, -1, [&response](cpp_redis::reply& reply) {
        if(reply.ok())
            response = reply.as_array();
    });
    
    client.sync_commit();
    
    DocumentTuple obtaineddoc(-1, "", "");
    
    if(response.size() == 5) {
        obtaineddoc.docID = stoi(response[0].as_string());
        obtaineddoc.doc = response[1].as_string();
        //Skip getting doclength
        obtaineddoc.timestamp = response[3].as_string();
    }
    
    return obtaineddoc;
}

void DocumentStore::insertDocument(std::string url, std::string doc, int termlength, string timestamp) {
    //Get nextid, olddoclen, avgdoclen, doccount
    string nextid;
    client.get("nextid", [&nextid](cpp_redis::reply& reply) {
        nextid = reply.as_string();
    });

    int olddoclen;
    client.lindex(url, 2, [&olddoclen](cpp_redis::reply& reply) {
        if(reply.is_null())
            olddoclen = -1;
        else
            olddoclen = stoi(reply.as_string());
    });

    client.sync_commit();

    double avgdoclen = getAverageDocLength();
    unsigned int doccount = getDocumentCount();

    //document doesn't exist
    if(olddoclen < 0) {
        vector<string> doctuple = {nextid, doc, to_string(termlength), timestamp};
        client.rpush(url, doctuple);

        client.select(2);
        client.set(nextid, url);
        client.select(0);

        client.incr("nextid");
        client.incr("doccount");
    }
    //document exists
    else {
        //Keep only the docid
        client.ltrim(url, 0, 0);
        vector<string> newdocinfo = {doc, to_string(termlength), timestamp};
        client.rpush(url, newdocinfo);

        avgdoclen = updateAverageRemove(avgdoclen, olddoclen, doccount);
        //Only need to do this for calculations
        doccount--;
    }

    avgdoclen = updateAverageAdd(avgdoclen, termlength, doccount);

    client.set("avgdoclen", to_string(avgdoclen));
    
    client.sync_commit();
}

void DocumentStore::dump() {
    //Ensure database is saved
    client.save();
    client.sync_commit();
}

size_t DocumentStore::getDocumentCount() {
    auto result = client.get("doccount");

    client.sync_commit();
    result.wait();

    return stoull(result.get().as_string());
}

int DocumentStore::getDocLength(unsigned int docID) {
    //Get url from docID
    client.select(2);
    auto result = client.get(to_string(docID));
    client.select(0);

    client.sync_commit();
    result.wait();

    string url = result.get().as_string();

    //Get doclength index
    result = client.lindex(url, 2);

    client.sync_commit();
    result.wait();

    return stoi(result.get().as_string());
}

double DocumentStore::getAverageDocLength() {
    auto result = client.get("avgdoclen");
    
    client.sync_commit();
    result.wait();

    return stod(result.get().as_string());
}

int DocumentStore::getNextDocID() {
    auto result = client.get("nextid");

    client.sync_commit();
    result.wait();

    return stoi(result.get().as_string());
}

void DocumentStore::clear() {
    client.flushdb();
    client.sync_commit();
}