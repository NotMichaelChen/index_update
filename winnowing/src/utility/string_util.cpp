#include "string_util.hpp"
#include <iomanip>
#include <cstring>
#include <sstream>

namespace StringUtil
{

//Get timestamp, https://stackoverflow.com/a/16358111
std::string getTimestamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    return oss.str();
}

//https://stackoverflow.com/a/3404820
//May not be optimal
unsigned int hashVector(const std::vector<int>& v) {
    unsigned int hc = v.size();
    for(size_t i = 0; i < v.size(); i++)
    {
        hc = hc*314159 + v[i];
    }
    return hc;
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

std::vector<std::string> splitString(const std::string& str, const std::string& delim) {
    std::vector<std::string> tokens;
    char* pMutableString = (char*) malloc( str.size()+1 );
    strcpy( pMutableString, str.c_str() );

    char* saveptr;
    char *p = strtok_r(pMutableString, delim.data(), &saveptr);
    while (p) {
        tokens.emplace_back(p);
        p = strtok_r(NULL, delim.data(), &saveptr);
    }
    free(pMutableString);

    return tokens;
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

//Returns whether the term has only alphanumerics or not
bool isAlnum(const std::string& s) {
    for(char i : s) {
        if(!isalnum(i))
            return false;
    }
    return true;
}

std::string vecToString(const std::vector<std::string>& vecString) {
    std::string res;
    for (auto w : vecString) {
        res += w;
        res += ' ';
    }
    return res;
}

}