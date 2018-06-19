#ifndef MORPH_HPP
#define MORPH_HPP

#include <string>
#include <sstream>

class DocumentMorpher {
public:
    DocumentMorpher(std::string from, std::string to, int numversions);

    std::string getVersion();
    bool isValid();

private:
    std::stringstream oldstream, newstream;
    int versioncount;
    //Used to calculate probability of switching documents when generating new versions
    int versionsleft;
};

#endif