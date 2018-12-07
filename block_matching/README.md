# Implementation for "Efficient Index Updates for Web Search Engines"

## Build instructions
Requirements:

* [Redis](https://redis.io/)
* [cpp-redis](https://github.com/Cylix/cpp_redis)
* [Meson](http://mesonbuild.com) + [Ninja](https://ninja-build.org/)

To build:

* Type `meson --buildtype {debug|debugoptimized|release} {builddir}`
  * This will create a build directory called `builddir`
* Change into the build directory and type `ninja` to build the program

To build tests:

* Type `meson -Dtest=true {builddir}`
* Change into the build directory and type `ninja` to build the tests

## Testing instructions

The program must be run against a script file as defined in `src/script_engine/readme.md`