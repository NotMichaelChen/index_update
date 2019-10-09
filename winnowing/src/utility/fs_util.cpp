#include "fs_util.hpp"

#include "dirent.h"
#include <stdexcept>

namespace FSUtil {

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

}