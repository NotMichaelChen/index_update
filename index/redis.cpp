#include "redis.hpp"

#include <fstream>

//Saves the current redis database using the given name
//Will be saved in the current working directory
void redisDumpDatabase(std::string name) {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);

    //Save to rdb file
    //Is synchronous, so it's fine to copy immediately after
    client.save();
    client.sync_commit();

    //Get location of rdb file
    //TODO: Returns array, fix access
    std::string path;
    client.config_get("dir", [&path](cpp_redis::reply& reply) {
        if(reply.ok())
            path = reply.as_string();
    });
    //Get name of rdb file
    std::string filename;
    client.config_get("dbfilename", [&filename](cpp_redis::reply& reply) {
        if(reply.ok())
            filename = reply.as_string();
    });

    client.sync_commit();

    //Copy the rdb file to the current directory
    std::ifstream src(path + "/" + filename, std::ios::binary);
    std::ofstream dst(name,   std::ios::binary);

    dst << src.rdbuf();

    client.disconnect();
}

void redisRestoreDatabase(std::string filepath) {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);

    //Get location of rdb file
    //Get name of rdb file
    //Move the given file to the specified directory
    //Change permissions of move file
    //Call shutdown with no save options
    //Assume database is running with daemon, and will restart when shutdown
}

void redisSaveDatabase() {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);
    client.sync_commit();
    client.disconnect();
}