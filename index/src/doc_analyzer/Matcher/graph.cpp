#include "graph.h"

#include <algorithm>

using namespace std;

BlockGraph::BlockGraph(vector<Block>& commonblocks) {
    //Sort based on old locations
    sort(commonblocks.begin(), commonblocks.end(), compareOld);
    
    //Create adjacency list for every block
    for(auto iter = commonblocks.begin(); iter != commonblocks.end(); iter++) {
        auto insertediter = G.emplace(make_pair(*iter, vector<Block>{})).first;
        
        //All potential neighbors will be strictly after the current block
        for(auto neighboriter = iter+1; neighboriter != commonblocks.end(); neighboriter++) {
            if(neighboriter->oldloc > iter->oldendloc() && neighboriter->newloc > iter->newendloc())
                insertediter->second.push_back(*neighboriter);
        }
    }
    
    vertices = commonblocks;
}

const vector<Block>& BlockGraph::getAdjacencyList(Block V) {
    return G[V];
}

const vector<Block>& BlockGraph::getAllVertices() const {
    return vertices;
}

//Create a topological ordering of graph via DFS
vector<Block> topologicalSort(BlockGraph& graph) {
    vector<Block> toporder;
    unordered_set<Block> visited;
    const vector<Block> vertices = graph.getAllVertices();
    for(Block i : vertices)
        explore(graph, i, visited, toporder);
    
    reverse(toporder.begin(), toporder.end());
    return toporder;
}

//Helper function for topologicalSort, explores all vertices connected to current
void explore(BlockGraph& graph, Block current, unordered_set<Block>& visited, vector<Block>& toporder) {
    //do not explore if already visited
    if(visited.find(current) != visited.end())
        return;
    visited.insert(current);
    vector<Block> adjacent = graph.getAdjacencyList(current);
    for(Block i : adjacent) {
        //Not visited yet
        if(visited.find(i) == visited.end())
            explore(graph, i, visited, toporder);
    }
    toporder.push_back(current);
}