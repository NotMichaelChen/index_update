#include "libs/catch.hpp"

#include <cpp_redis/cpp_redis>
#include <iostream>

#include "utility/redis.hpp"

TEST_CASE("Test redis dump/restore", "[redis]") {
    redis_flushDB();

    cpp_redis::client client;
    client.connect("127.0.0.1", 6379);

    client.set("foo", "bar");
    client.sync_commit();

    std::string filename = "testfile";
    redis_dump(filename, 0);

    redis_flushDB();

    // Key should no longer exist
    auto fut = client.exists({"foo"});
    client.sync_commit();
    fut.wait();
    REQUIRE(fut.get().as_integer() == 0);

    redis_restore(filename, 0);

    // Key should be restored
    fut = client.get("foo");
    client.sync_commit();
    fut.wait();
    REQUIRE(fut.get().as_string() == "bar");

    client.disconnect();
}