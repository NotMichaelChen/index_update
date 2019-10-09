# Implementations for "Efficient Index Updates for Web Search Engines"

## Overview

This repository houses three implementations of search engines that support efficient document updates.

* "block-matching" is a new approach that avoids pre-partitioning documents. Instead, when document updates arrive, translation mappings are generated which represent how certain parts of the document move around, allowing old positional postings to be transformed to represent their new positions.
* The "[landmarks](http://wwwconference.org/proceedings/www2003/papers/refereed/p656/p656-lim.html)" approach involves inserting "landmarks" into each document, then representing positions of terms relative to each landmark. Thus insertions/deletions only affect the positions of the landmarks rather than the positions of all words.
* The "[winnowing](http://wwwconference.org/www2007/papers/paper800.pdf)" approach splits each document into fragments, then only indexes unique fragments. Thus when a document is updated, only changed fragments need to be updated.

## Build instructions

Requirements:

* [Redis](https://redis.io/)
* [cpp-redis](https://github.com/cpp-redis/cpp_redis)
* [CMake](https://cmake.org/) + [Make](https://www.gnu.org/software/make/)

Inside each folder is a `CMakeLists.txt` which builds each project.

To build:

* Starting from the root of any of the three project folders, type these commands into the terminal:
  * `mkdir build`
  * `cd build`
  * `cmake ..`
  * `make`
* To create a `Release` build (optimized), run `cmake -DCMAKE_BUILD_TYPE=Release ..`
* To create an optimized build with debug symbols, run `cmake -DCMAKE_BUILD_TYPE=RelWithDebugInfo ..`

## Outdated Instructions below

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