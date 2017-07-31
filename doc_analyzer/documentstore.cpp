#include "documentstore.h"

#include <cpp_redis/cpp_redis>
#include <time.h>

#ifdef _WIN32
#include <Winsock2.h>
#endif /* _WIN32 */

using namespace std;

DocumentStore::DocumentStore() : nextid(0) {
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
}

DocumentTuple DocumentStore::getDocument(string url) {
    vector<cpp_redis::reply> response;
    
    client.lrange(url, 0, -1, [&response](cpp_redis::reply& reply) {
        if(reply.ok())
            response = reply.as_array();
    });
    
    client.sync_commit();
    
    DocumentTuple obtaineddoc(-1, "", 0, "");
    
    if(response.size() == 4) {
        obtaineddoc.docID = stoi(response[0].as_string());
        obtaineddoc.doc = response[1].as_string();
        obtaineddoc.maxfragID = stoi(response[2].as_string());
        obtaineddoc.timestamp = response[3].as_string();
    }
    
    return obtaineddoc;
}

void DocumentStore::insertDocument(std::string url, std::string doc, unsigned int maxfragID, string timestamp) { 
    vector<string> keys = {url};
    client.exists(keys, [&](cpp_redis::reply& reply) {
        if(reply.ok()) {
            //If the key already exists
            if(reply.as_integer()) {
                //Keep only the docid
                client.ltrim(url, 0, 0);
                vector<string> newdocinfo = {doc, to_string(maxfragID), timestamp};
                client.rpush(url, newdocinfo);
            }
            else {
                vector<string> doctuple = {to_string(nextid), doc, to_string(maxfragID), timestamp};
                client.rpush(url, doctuple);
                ++nextid;
            }
            
            client.commit();
        }
        
        //Do nothing if the exists command returns an error
    });
    
    client.sync_commit();
}

int DocumentStore::getNextDocID() {
    return nextid;
}