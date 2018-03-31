#include "dirent.h"

#include <fstream>
#include <chrono>

#include "index.hpp"

using namespace std;

//http://forum.codecall.net/topic/60157-read-all-files-in-a-folder/
vector<string> openInDir(string path = ".") {
    DIR*    dir;
    dirent* pdir;
    vector<string> files;

    dir = opendir( path.empty() ? "." : path.c_str() );

    if( dir ){
        while (true){
  			pdir = readdir( dir );
  			if (pdir == NULL) break;
            string d_n(pdir->d_name);
  			files.push_back( d_n );
      	}
    	closedir( dir );
    }
    else cout << "Directory not opened." << endl;

    return files;
}

int main(int argc, char **argv) {
    //Reset the redis database before testing
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);
    client.flushall();
    client.sync_commit();
    client.disconnect();

    Index index;
    index.restorelex();
    cin.ignore();
    vector<string> filelist = openInDir("./dataset-format/");

    auto begin = chrono::high_resolution_clock::now();
    int docs = 0;
    for(string& i : filelist) {

        if(i == "." || i == "..")
            continue;
        string filename;
        //Split on ~, everything before is filename
        for(auto iter = i.begin(); iter != i.end(); iter++) {
            if(*iter == '~') {
                filename = string(i.begin(), iter);
                break;
            }
        }

        ifstream inputfile("./dataset-format/" + i);
        //https://www.reddit.com/r/learnprogramming/comments/3qotqr/how_can_i_read_an_entire_text_file_into_a_string/cwh8m4d/
        string filecontents{ istreambuf_iterator<char>(inputfile), istreambuf_iterator<char>() };

        cout << "Inserting file: " << i << endl;
        index.insert_document(filename, filecontents);
        ++docs;
    }

    index.dump();

    auto end = chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    std::cout << "Inserted " << docs << " documents in " << ms << "ms for an average of " << ms / (double)docs << " ms/doc\n";

    return 0;
}
