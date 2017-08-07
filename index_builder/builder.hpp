#ifndef BUILDER_H
#define BUILDER_H

#include <string>
#include <vector>
#include <sstream>
#include "strless.hpp"

class Builder{
public:
    void build_lexical(std::map<std::string, unsigned int, strless>);
    void display_lexical(std::map<std::string, unsigned int, strless>);
};

#endif
