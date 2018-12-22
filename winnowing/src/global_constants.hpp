#ifndef GLOBAL_CONSTANTS_HPP
#define GLOBAL_CONSTANTS_HPP

#define MIN_BLOCK_SIZE 10
#define MAX_BLOCK_COUNT 100

#define POSTING_LIMIT 10000000 //make sure doesn't exceed memory limit 10000000
#define BLOCKSIZE 128

#define DAAT_SIZE 10

//How many postings are required to get an entry into the extended lexicon
#define SPARSE_SIZE 100
//How many postings must be accumulated without a big entry to insert another pointer
#define SPARSE_BETWEEN_SIZE 100

#include <limits>
#include <string>

const auto UIntMax = std::numeric_limits<unsigned int>::max();

//Partial paths to static indexes folders
const std::string IndexPath = "/disk_index/";
const std::string PosPath = "/disk_index/positional/";
const std::string NonPosPath = "/disk_index/non_positional/";

#endif