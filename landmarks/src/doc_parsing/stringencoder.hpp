#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <unordered_set>

//Encodes two versions of a file into lists of integers.
//Also stores information about terms exclusive to either version
class StringEncoder {
public:
    StringEncoder(std::string oldfile, std::string newfile);
    
    //Decodes a stream of integers into a list of words
    //Unknown ints are replaced with ??
    std::vector<std::string> decodeStream(std::vector<unsigned int>& stream);
    //Decodes an individual integer into a single word
    std::string decodeNum(unsigned int num);

    //Get the count of a word in the new document
    int getNewCount(std::string word);

    //Sees if a word is in the old document or new document
    bool inOld(std::string& word);
    bool inNew(std::string& word);
    
    //Iterators for accessing the encoded documents
    std::vector<int>::const_iterator getOldIter();
    std::vector<int>::const_iterator getNewIter();
    std::vector<int>::const_iterator getOldEnd();
    std::vector<int>::const_iterator getNewEnd();

    //Accessing the size of the encoded documents
    int getOldSize();
    int getNewSize();

    const std::vector<int>& getOldEncoded();
    const std::vector<int>& getNewEncoded();
    
private:
    //old and new files in integer form
    std::vector<int> oldencoded;
    std::vector<int> newencoded;
    //lists of terms that appear only in the old or new file
    std::unordered_set<std::string> oldexclusive;
    std::unordered_set<std::string> newexclusive;
    //list of terms that appear in the new document and their frequency
    std::unordered_map<std::string, int> newcount;
    
    //Maps words to numbers
    std::unordered_map<std::string, int> dictionary;
    //Maps numbers (index) to words
    //Since id's are assigned incrementally, a vector can be used for lookup
    //Should be appended whenever nextcode is incremented
    std::vector<std::string> lookup;
    //Represents next unique identifier
    unsigned int nextcode;
};