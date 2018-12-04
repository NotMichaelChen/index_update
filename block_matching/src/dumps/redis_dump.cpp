#include "redis_dump.hpp"

RedisDump::RedisDump(std::string& dir) : workingdir(dir) {}

void RedisDump::dump(std::string& name) {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);

    std::ofstream ofile(name, std::ios::binary);

    size_t cursor = 0;
    std::vector<cpp_redis::reply> keys;

    do {
        client.scan(cursor, [&cursor, &keys](cpp_redis::reply& reply) {
            std::vector<cpp_redis::reply> response = reply.as_array();
            cursor = response[0].as_integer();
            keys = response[1].as_array();
        });

        client.sync_commit();

        for(auto reply : keys) {
            std::string k = reply.as_string();

            auto result = client.dump(k);
            result.wait();
            std::string val = result.get().as_string();

            writeStringPair(ofile, k, val);
        }

    } while(cursor != 0);

    ofile.close();
    client.disconnect();
}

void RedisDump::restore(std::string& filename) {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);

    std::ifstream ifile(filename, std::ios::binary);

    std::string key;
    std::string hash;

    while(readStringPair(ifile, key, hash)) {
        client.restore(key, 0, hash);
        client.sync_commit();
    }

    ifile.close();
    client.disconnect();
}

void RedisDump::flushDB() {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);
    client.flushall();
    client.sync_commit();
    client.disconnect();
}

void RedisDump::writeStringPair(std::ofstream& ofile, std::string& key, std::string& hash) {
    size_t keysize = key.size();
    size_t hashsize = hash.size();
    ofile.write(reinterpret_cast<const char*>(&keysize), sizeof(keysize));
    ofile << key;
    ofile.write(reinterpret_cast<const char*>(&hashsize), sizeof(hashsize));
    ofile << hash;
}

bool RedisDump::readStringPair(std::ifstream& ifile, std::string& key, std::string& hash) {
    
}