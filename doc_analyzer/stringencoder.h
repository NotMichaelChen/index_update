#ifndef STRINGENCODER_H
#define STRINGENCODER_H

#include <string>
#include <vector>
#include <fstream>
#include <map>

//Encodes a file of words into a list of integers
class StringEncoder {
public:
    StringEncoder();
    std::vector<int> encodeFile(std::ifstream& file);
    std::vector<std::string> decodeStream(std::vector<int>& stream);
    std::string decodeNum(int num);
    
private:
    //Assigns words to numbers
    std::map<std::string, int> dictionary;
    //Since id's are assigned incrementally, a vector can be used for lookup
    //Should be appended whenever nextcode is incremented
    std::vector<std::string> lookup;
    //Represents next unique identifier
    int nextcode;
};

#endif