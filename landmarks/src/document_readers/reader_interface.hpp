#ifndef READER_INTERFACE_HPP
#define READER_INTERFACE_HPP

#include <string>

class ReaderInterface {
public:
    //All readers should already have a valid currentDocument after initialization

    //Returns the url of the current document
    virtual std::string getURL() = 0;
    //Returns the current document of the reader
    virtual std::string getCurrentDocument() = 0;
    //Returns whether a next document was sucessfully gotten
    virtual bool nextDocument() = 0;
    //Returns whether the document reader is in a valid state
    virtual bool isValid() = 0;
};

#endif