#ifndef FS_UTIL_HPP
#define FS_UTIL_HPP

#include <vector>
#include <string>

namespace FSUtil {

    //Gets a list of files in a given directory
    std::vector<std::string> readDirectory(std::string path);

}

#endif