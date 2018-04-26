#include "util.hpp"

#include "dirent.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>

namespace Utility
{

//Get timestamp, https://stackoverflow.com/a/16358111
std::string getTimestamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    return oss.str();
}

//http://forum.codecall.net/topic/60157-read-all-files-in-a-folder/
std::vector<std::string> readDirectory(std::string path = ".") {
    DIR*    dir;
    dirent* pdir;
    std::vector<std::string> files;

    dir = opendir( path.empty() ? "." : path.c_str() );

    if( dir ){
        while (true){
  			pdir = readdir( dir );
  			if (pdir == NULL) break;
            std::string d_n(pdir->d_name);
  			files.push_back( d_n );
      	}
    	closedir( dir );
    }
    else throw "Directory not opened.";

    return files;
}

}