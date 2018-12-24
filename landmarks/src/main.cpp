#include <iostream>

#include "document_readers/WETreader.hpp"
#include "index/index.hpp"
#include "utility/redis.hpp"
#include "utility/morph.hpp"
#include "utility/timer.hpp"

int main(int argc, char const *argv[])
{
    redis_flushDB();
    Index index("index_files");
    WETReader wr("CC");
    Utility::Timer timer;

    std::string lastdoc = wr.getCurrentDocument();
    std::string lasturl = wr.getURL();
    wr.nextDocument();
    timer.start();
    while (wr.isValid()) {
        std::string currdoc = wr.getCurrentDocument();
        std::string currurl = wr.getURL();
        DocumentMorpher dm(lastdoc, currdoc, 10);
        std::cout << "==========\n" << currurl << '\n';
        while (dm.isValid()) {
            std::string doc = dm.getDocument();
            index.insert_document(currurl, doc);
            dm.nextVersion();
        }
        lastdoc = currdoc;
        lasturl = currurl;
        wr.nextDocument();
    }
    timer.stop();

    std::cerr << timer.getCumulative() << "ms" << std::endl;
    index.printSize();

    return 0;
}
