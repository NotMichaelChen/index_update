#include "util.hpp"

#include "dirent.h"
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

//Gets all files inside the given directory
//Returns a vector of *only* the file names
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
            //ignore ". and .."
            if(d_n == "." || d_n == "..")
                continue;
  			files.push_back( d_n );
      	}
    	closedir( dir );
    }
    else throw std::runtime_error("Directory not opened.");

    return files;
}

//https://stackoverflow.com/a/10058725
std::vector<std::string> splitString(std::string& str, char c) {
    std::stringstream stream(str);
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(stream, segment, c))
        seglist.push_back(segment);
    
    return seglist;
}

//https://codereview.stackexchange.com/a/40302
std::string trim(std::string const& str)
{
    if(str.empty())
        return str;

    std::size_t firstScan = str.find_first_not_of(' ');
    std::size_t first     = firstScan == std::string::npos ? str.length() : firstScan;
    std::size_t last      = str.find_last_not_of(' ');
    return str.substr(first, last-first+1);
}

}