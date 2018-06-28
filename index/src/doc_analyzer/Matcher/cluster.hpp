#pragma once

#include "block.h"

//A cluster is defined to be a set of blocks that all have the same adjacency list
struct Cluster {
    Cluster(std::vector<std::shared_ptr<Block>> v, int m) : vertices(v), max_oldlocend(m) {}

    std::vector<std::shared_ptr<Block>> vertices;
    int max_oldlocend;
};

//Less-than comparison
bool compareCluster(const Cluster& lhs, const Cluster& rhs);

std::vector<Cluster> generateClusters(std::vector<std::shared_ptr<Block>>& commonblocks);