#include "redis.hpp"

#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <fstream>
#include <vector>

void do_chown(const char *file_path, const char *user_name, const char *group_name) {
    uid_t          uid;
    gid_t          gid;
    struct passwd *pwd;
    struct group  *grp;

    pwd = getpwnam(user_name);
    if (pwd == NULL) {
        throw std::invalid_argument("Failed to get uid");
    }
    uid = pwd->pw_uid;

    grp = getgrnam(group_name);
    if (grp == NULL) {
        throw std::invalid_argument("Failed to get gid");
    }
    gid = grp->gr_gid;

    if (chown(file_path, uid, gid) == -1) {
        throw std::invalid_argument("chown fail");
    }
}

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
    std::string path;
    client.config_get("dir", [&path](cpp_redis::reply& reply) {
        if(reply.ok()) {
            std::vector<cpp_redis::reply> response;
            if(response.size() >= 2)
                path = reply.as_array()[1].as_string();
        }
    });
    //Get name of rdb file
    std::string filename;
    client.config_get("dbfilename", [&filename](cpp_redis::reply& reply) {
        if(reply.ok()) {
            std::vector<cpp_redis::reply> response;
            if(response.size() >= 2)
                filename = reply.as_array()[1].as_string();
        }
    });

    client.sync_commit();

    //Copy the rdb file to the current directory
    std::ifstream src(path + "/" + filename, std::ios::binary);
    std::ofstream dst(name, std::ios::binary);

    dst << src.rdbuf();

    client.disconnect();
}

void redisRestoreDatabase(std::string filepath) {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);

    //Get location of rdb file
    std::string path;
    client.config_get("dir", [&path](cpp_redis::reply& reply) {
        if(reply.ok()) {
            std::vector<cpp_redis::reply> response;
            if(response.size() >= 2)
                path = reply.as_array()[1].as_string();
        }
    });
    //Get name of rdb file
    std::string filename;
    client.config_get("dbfilename", [&filename](cpp_redis::reply& reply) {
        if(reply.ok()) {
            std::vector<cpp_redis::reply> response;
            if(response.size() >= 2)
                filename = reply.as_array()[1].as_string();
        }
    });

    client.sync_commit();

    //Copy the rdb file to the current directory
    std::ifstream src(filepath, std::ios::binary);
    std::ofstream dst(path + "/" + filename, std::ios::binary);

    dst << src.rdbuf();

    //Change permissions of copied file
    std::string movedfilepath = path + "/" + filename;
    do_chown(movedfilepath.c_str(), "redis", "redis");
    //Call shutdown with no save options
    //Assume database is running with daemon, and will restart when shutdown
    client.shutdown("nosave");
    client.sync_commit();
}

void redisFlushDatabase() {
    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);
    client.flushall();
    client.sync_commit();
    client.disconnect();
}