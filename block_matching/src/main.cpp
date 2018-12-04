#include "script_engine/parse_engine.hpp"

#include <iostream>
#include <cpp_redis/cpp_redis>

using namespace std;

int main(int argc, char **argv) {
    if(argc != 2) {
        cout << "Usage: ./index scriptpath" << endl;
        return 1;
    }
    
    //Reset the redis database before testing
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);
    client.flushall();
    client.sync_commit();
    client.disconnect();

    parseFile(argv[1]);

    // Index index;
    // WETReader reader("CC");

    // auto begin = chrono::high_resolution_clock::now();
    
    // uintmax_t doccount = 0;

    // do
    // {
    //     std::string url = reader.getURL();
    //     std::string contents = reader.getCurrentDocument();

    //     cout << "Inserting file: " << url << endl;

    //     index.insert_document(url, contents);
    //     doccount++;

    // } while(reader.nextDocument());

    // auto end = chrono::high_resolution_clock::now();
    // auto dur = end - begin;
    // auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    // std::cout << "Inserted " << doccount << " documents in " << ms << "ms for an average of " << ms / (double)doccount << " ms/doc\n";

    return 0;
}
