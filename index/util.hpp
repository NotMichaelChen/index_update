#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
#include <string>

namespace Utility {

    //Gets a timestamp of the current time formatted as a string
    std::string getTimestamp();

    //Gets a list of files in a given directory
    std::vector<std::string> readDirectory(std::string path = ".");

    template<typename T>
    void binaryInsert(std::vector<T>& array, T element);

    #include "util.tcc"
}

#endif