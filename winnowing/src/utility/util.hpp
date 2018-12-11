#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
#include <string>

namespace Utility {

    //Gets a timestamp of the current time formatted as a string
    std::string getTimestamp();

    //Hashes a vector of ints
    //Not guaranteed to be optimal
    unsigned int hashVector(const std::vector<int>& v);

    //Gets a list of files in a given directory
    std::vector<std::string> readDirectory(std::string path);

    std::vector<std::string> splitString(std::string& str, char c);
    std::vector<std::string> splitString(const std::string& str, const std::string& delim);

    std::string trim(std::string const& str);

    bool isAlnum(const std::string& s);
}

#endif