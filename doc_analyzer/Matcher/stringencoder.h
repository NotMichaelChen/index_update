#ifndef STRINGENCODER_H
#define STRINGENCODER_H

#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <map>

namespace Matcher {
    //Encodes two versions of a file into lists of integers.
    //Also stores information about terms exclusive to either version
    class StringEncoder {
    public:
        StringEncoder(std::string& oldfile, std::string& newfile);
        
        //Decodes a stream of integers into a list of words
        //Unknown ints are replaced with ??
        std::vector<std::string> decodeStream(std::vector<int>& stream);
        //Decodes an individual integer into a single word
        std::string decodeNum(int num);
        
        //Iterators for accessing the encoded documents
        std::vector<int>::const_iterator getOldIter();
        std::vector<int>::const_iterator getNewIter();
        std::vector<int>::const_iterator getOldEnd();
        std::vector<int>::const_iterator getNewEnd();
    
        //Accessing the size of the encoded documents
        int getOldSize();
        int getNewSize();
        
    private:
        //old and new files in integer form
        std::vector<int> oldencoded;
        std::vector<int> newencoded;
        //lists of terms that appear only in the old or new file
        std::vector<std::string> oldexclusive;
        std::vector<std::string> newexclusive;
        //Maps words to numbers
        std::unordered_map<std::string, int> dictionary;
        //Maps numbers (index) to words
        //Since id's are assigned incrementally, a vector can be used for lookup
        //Should be appended whenever nextcode is incremented
        std::vector<std::string> lookup;
        //Represents next unique identifier
        int nextcode;
    };
}

#endif