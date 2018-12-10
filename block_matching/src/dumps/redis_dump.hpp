#pragma once

#include <cpp_redis/cpp_redis>

#include <string>
#include <fstream>

void redis_dump(std::string& name, int dbnum);
void redis_restore(std::string& filename, int dbnum);
void redis_flushDB();