#pragma once

#include <cpp_redis/cpp_redis>

#include <string>
#include <fstream>

class RedisDump {
public:
    RedisDump(std::string& dir);
    
    void dump(std::string& filename);
    void restore(std::string& filename);
    void flushDB();

private:
    void writeStringPair(std::ofstream& ofile, std::string& key, std::string& hash);
    bool readStringPair(std::ifstream& ifile, std::string& key, std::string& hash);

    std::string workingdir;
};