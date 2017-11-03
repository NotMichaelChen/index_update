#include "translationtable.h"

#include <vector>
#include <unordered_map>
#include <sstream>
#include <cpp_redis/cpp_redis>

#ifdef _WIN32
#include <Winsock2.h>
#endif /* _WIN32 */

#include "../doc_analyzer/Matcher/translate.h"

using namespace std;

namespace Structures {
    string transToString(Matcher::Translation& t);
    Matcher::Translation stringToTrans(const string& s);
    
    TranslationTable::TranslationTable() {
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
        client.select(1);
    }
    
    int TranslationTable::apply(int docID, int fragID, int position) {
        vector<cpp_redis::reply> response;
        
        client.lrange(to_string(docID), 0, -1, [&response](cpp_redis::reply& reply) {
            if(reply.ok())
                response = reply.as_array();
        });
        
        client.sync_commit();
        
        if(response.size() == 0)
            return -1;
        
        int finalposition = position;
        for(int i = fragID; i < response.size(); ++i) {
            finalposition = Matcher::applyTranslation(finalposition, stringToTrans(response[i].as_string()));
            //return if position becomes invalidated
            //cannot keep running; position might accidentally become revalidated
            if(finalposition < 0)
                return finalposition;
        }
        
        return finalposition;
    }
    
    void TranslationTable::insert(vector<Matcher::Translation>& trans, int docID) {
        vector<string> val;
        for(Matcher::Translation& t : trans)
            val.push_back(transToString(t));
        
        client.rpush(to_string(docID), val);
        client.commit();
    }

    void TranslationTable::dump() {
        //Ensure database is saved
        client.save();
        client.sync_commit();
    }
    
    void TranslationTable::erase(int docID) {
        client.del( {to_string(docID)} );
        client.commit();
    }
    
    string transToString(Matcher::Translation& t) {
        stringstream result;
        result << t.loc << "-" << t.oldlen << "-" << t.newlen;
        return result.str();
    }
    
    Matcher::Translation stringToTrans(const string& s) {
        int nums[3];
        stringstream stringtrans(s);
        string token;
        int i = 0;
        while(getline(stringtrans, token, '-')) {
            nums[i] = stoi(token);
            ++i;
        }
        
        return Matcher::Translation(nums[0], nums[1], nums[2]);
    }
}
