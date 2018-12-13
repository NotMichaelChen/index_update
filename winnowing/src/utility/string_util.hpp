#ifndef STRING_UTIL_HPP
#define STRING_UTIL_HPP

#include <vector>
#include <string>

namespace StringUtil {

    //Gets a timestamp of the current time formatted as a string
    std::string getTimestamp();

    //Hashes a vector of ints
    //Not guaranteed to be optimal
    unsigned int hashVector(const std::vector<int>& v);

    std::vector<std::string> splitString(std::string& str, char c);
    std::vector<std::string> splitString(const std::string& str, const std::string& delim);

    std::string trim(std::string const& str);

    bool isAlnum(const std::string& s);

    std::string vecToString(const std::vector<std::string>& vecString);
}

#endif