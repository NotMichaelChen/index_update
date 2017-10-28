#ifndef EXTLEX_H
#define EXTLEX_H

#include <string>
#include "meta.hpp"

class ExtendedLexicon{

public:
    void addPositional();
    void addNonPositional();
private:
    std::map<unsigned int, std::vector<mDatanp>> exlexnp;
    std::map<unsigned int, std::vector<mDatap>> exlexp;
}

#endif
