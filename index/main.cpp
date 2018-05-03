#include "dirent.h"

#include <fstream>
#include <chrono>

#include "index.hpp"
#include "redis.hpp"
#include "util.hpp"
#include "document_readers/WETreader.hpp"

using namespace std;

int main(int argc, char **argv) {
    //Reset the redis database before testing
    redisFlushDatabase();

    Index index;
    WETReader reader("CC");

    auto begin = chrono::high_resolution_clock::now();
    
    uintmax_t doccount = 0;

    std::cerr << "begin loop\n";
    do
    {
        std::string url = reader.getURL();
        std::string contents = reader.getCurrentDocument();

        cout << "Inserting file: " << url << endl;

        index.insert_document(url, contents);
        doccount++;

    } while(reader.nextDocument());

    auto end = chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    std::cout << "Inserted " << doccount << " documents in " << ms << "ms for an average of " << ms / (double)doccount << " ms/doc\n";

    return 0;
}
