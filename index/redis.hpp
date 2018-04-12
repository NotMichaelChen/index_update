#ifndef REDIS_HPP
#define REDIS_HPP

#include <cpp_redis/cpp_redis>

void redisDumpDatabase(std::string name);
void redisRestoreDatabase(std::string filepath);
void redisFlushDatabase();

#endif