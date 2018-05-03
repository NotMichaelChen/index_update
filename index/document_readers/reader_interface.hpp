#ifndef READER_INTERFACE_HPP
#define READER_INTERFACE_HPP

#include <string>

class ReaderInterface {
public:
    virtual std::string getCurrentDocument() = 0;
    virtual bool nextDocument() = 0;
};

#endif