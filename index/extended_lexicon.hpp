#ifndef EXTLEX_H
#define EXTLEX_H

#include <string>
#include "meta.hpp"

class ExtendedLexicon {
public:
    void addPositional();
    void addNonPositional();
private:
    std::map<std::string, std::vector<mDatanp>> exlexnp;
    std::map<std::string, std::vector<mDatap>> exlexp;
}

#endif
