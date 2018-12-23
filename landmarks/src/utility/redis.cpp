#include "redis.hpp"

#include <cstdint>

#include "static_functions/bytesIO.hpp"
#include <iostream>

static void writeStringPair(std::ofstream& ofile, std::string& key, std::string& hash) {
    size_t keysize = key.size();
    size_t hashsize = hash.size();
    writeAsBytes(keysize, ofile);
    ofile << key;
    writeAsBytes(hashsize, ofile);
    ofile << hash;
}

static bool readStringPair(std::ifstream& ifile, std::string& key, std::string& hash) {
    size_t keysize = 0;
    size_t hashsize = 0;
    std::vector<char> buf;

    readFromBytes(keysize, ifile);
    buf.resize(keysize);
    ifile.read(&buf.front(), keysize);
    key = std::string(buf.begin(), buf.end());

    readFromBytes(hashsize, ifile);
    buf.resize(hashsize);
    ifile.read(&buf.front(), hashsize);
    hash = std::string(buf.begin(), buf.end());

    return (bool) ifile;
}

void redis_dump(std::string& name, int dbnum) {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);
    client.select(dbnum);
    client.sync_commit();

    std::ofstream ofile(name, std::ios::binary);

    int64_t cursor = 0;
    std::vector<cpp_redis::reply> keys;

    do {
        // Scan for keys page-by-page
        client.scan(cursor, [&cursor, &keys](cpp_redis::reply& reply) {
            std::vector<cpp_redis::reply> response = reply.as_array();
            cursor = stoll(response[0].as_string());
            keys = response[1].as_array();
        });
        client.sync_commit();

        // For each key:
        for(auto reply : keys) {
            std::string k = reply.as_string();

            // Get its value
            auto result = client.dump(k);
            client.sync_commit();
            result.wait();
            std::string val = result.get().as_string();

            // Write out key-value pair
            writeStringPair(ofile, k, val);
        }

    } while(cursor != 0);

    ofile.close();
    client.disconnect();
}

void redis_restore(std::string& filename, int dbnum) {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);
    client.select(dbnum);
    client.sync_commit();

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

void redis_flushDB() {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);
    client.flushall();
    client.sync_commit();
    client.disconnect();
}