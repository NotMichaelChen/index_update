#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>

namespace Utility {

    template<typename T>
    void binaryInsert(std::vector<T>& array, T element);

    #include "util.tcc"
}

#endif