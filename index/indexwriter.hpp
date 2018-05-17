#ifndef INDEXWRITER_HPP
#define INDEXWRITER_HPP

#include "index.hpp"

//Returns the name of the folder that was written to
void writeIndex(std::string name, Index& index);
void readIndex(std::string name, Index& index);

#endif