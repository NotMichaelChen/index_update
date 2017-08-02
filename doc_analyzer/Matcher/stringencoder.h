#ifndef STRINGENCODER_H
#define STRINGENCODER_H

#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <map>

//Encodes the two versions of a file into lists of integers

class StringEncoder {
public:
    StringEncoder(std::string& oldfile, std::string& newfile);
    
    std::vector<std::string> decodeStream(std::vector<int>& stream);
    std::string decodeNum(int num);
    
    std::vector<int>::const_iterator getOldIter();
    std::vector<int>::const_iterator getNewIter();
    
    int getOldSize();
    int getNewSize();
    
private:
    //old and new files in integer form
    std::vector<int> oldencoded;
    std::vector<int> newencoded;
    //lists of terms that appear only in the old or new file
    std::vector<int> oldexclusive;
    std::vector<int> newexclusive;
    //Maps words to numbers
    std::unordered_map<std::string, int> dictionary;
    //Maps numbers (index) to words
    //Since id's are assigned incrementally, a vector can be used for lookup
    //Should be appended whenever nextcode is incremented
    std::vector<std::string> lookup;
    //Represents next unique identifier
    int nextcode;
};

#endif