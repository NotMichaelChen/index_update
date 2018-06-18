# Implementation for "Efficient Index Updates for Web Search Engines"

## Build instructions
Requirements:

* [Redis](https://redis.io/)
* [cpp-redis](https://github.com/Cylix/cpp_redis)
* [Meson](http://mesonbuild.com) + [Ninja](https://ninja-build.org/)

To build:

* Create a directory to hold the build files (ex. `builddir`), and change into it
* Type `meson .. {debug|debugoptimized|release}` into the terminal in your created directory
* Type `ninja` to build the program

To build tests:

* Create a directory to hold the build files (can be different from your eariler folder) and change into it
* Type `meson .. -D:test=true` into the terminal
* Type `ninja` to build the tests

## Testing instructions

The program must be run against a script file as defined in `src/script_engine/readme.md`