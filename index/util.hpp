#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
#include <string>

namespace Utility {

    std::string getTimestamp();

    template<typename T>
    void binaryInsert(std::vector<T>& array, T element);

    #include "util.tcc"
}

#endif