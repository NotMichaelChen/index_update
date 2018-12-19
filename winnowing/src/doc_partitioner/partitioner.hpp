#ifndef PARTITIONER_H
#define PARTITIONER_H

#include "global_types.hpp"

#include <vector>
#include <string>

// Implements the content-dependent partitioning approach based on winnowing
class Partitioner {
private:
    const unsigned int B_WINNOW;
    const unsigned int W_WINNOW;

    std::vector<std::string> normalizeFile(const std::string& origFile);

    std::vector<unsigned char> wordHashFile(const std::vector<std::string>& normalized);

    std::vector<size_t> hashFile(const std::vector<unsigned char>& file);

    std::vector<size_t> cutFile(const std::vector<size_t>& hashedFile);
public:
    Partitioner(const unsigned int b, const unsigned int w);

    std::vector<Fragment> partitionPage(size_t did, const std::string& origPage);
};

#endif
