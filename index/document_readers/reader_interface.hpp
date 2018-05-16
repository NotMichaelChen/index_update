#ifndef READER_INTERFACE_HPP
#define READER_INTERFACE_HPP

#include <string>

class ReaderInterface {
public:
    //Returns the current document of the reader
    virtual std::string getCurrentDocument() = 0;
    //Returns whether a next document was sucessfully gotten
    virtual bool nextDocument() = 0;
};

#endif