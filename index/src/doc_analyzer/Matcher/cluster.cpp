#include "cluster.hpp"

#include <algorithm>

bool compareCluster(const Cluster& lhs, const Cluster& rhs) {
    return lhs.max_oldlocend < rhs.max_oldlocend;
}

std::shared_ptr<Block> getImmediateNeighbor(std::vector<std::shared_ptr<Block>>& commonblocks, size_t index) {
    std::shared_ptr<Block>& source = commonblocks[index];
    
    //All potential neighbors will be strictly after the current block
    for(auto neighborindex = index+1; neighborindex < commonblocks.size(); neighborindex++) {
        std::shared_ptr<Block>& neighbor = commonblocks[neighborindex];
        //Return the first neighbor encountered
        if(neighbor->oldloc > source->oldendloc() && neighbor->newloc > source->newendloc())
            return neighbor;
    }

    return nullptr;
}

std::vector<Cluster> generateClusters(std::vector<std::shared_ptr<Block>>& commonblocks) {
    std::sort(commonblocks.begin(), commonblocks.end(), compareOld);

    std::unordered_map<std::shared_ptr<Block>, std::vector<std::shared_ptr<Block>>> cluster_table;
    //Stores blocks with no neighbors
    std::vector<std::shared_ptr<Block>> no_neighbors;

    //Build clusters
    for(size_t i = 0; i < commonblocks.size(); i++) {
        std::shared_ptr<Block> immediate_neighbor = getImmediateNeighbor(commonblocks, i);
        if(immediate_neighbor == nullptr) {
            no_neighbors.push_back(commonblocks[i]);
        }
        
        cluster_table[immediate_neighbor].push_back(commonblocks[i]);
    }

    std::vector<Cluster> clustervector;

    //Flatten map
    for(auto iter = cluster_table.begin(); iter != cluster_table.end(); iter++) {
        int max_oldlocend = -1;
        for(std::shared_ptr<Block>& v : iter->second) {
            if(v->oldendloc() > max_oldlocend)
                max_oldlocend = v->oldendloc();
        }

        clustervector.emplace_back(iter->second, max_oldlocend);
    }
    //Add no-neighbor blocks as their own clusters
    for(auto iter = no_neighbors.begin(); iter != no_neighbors.end(); iter++) {
        clustervector.emplace_back(std::vector<std::shared_ptr<Block>>{*iter}, (*iter)->oldendloc());
    }

    return clustervector;
}