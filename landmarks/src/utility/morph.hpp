#ifndef MORPH_HPP
#define MORPH_HPP

#include <string>
#include <sstream>
#include <random>

class DocumentMorpher {
public:
    DocumentMorpher(std::string& from, std::string& to, int numversions);

    std::string getDocument();
    void nextVersion();

    bool isValid();

private:
    std::string olddoc, newdoc;

    //Used to calculate probability of switching documents when generating new versions
    int versionsleft;

    std::mt19937_64 gen;
    std::uniform_real_distribution<double> dis;
};

#endif