#include "graph.h"

#include <algorithm>
#include <iostream>

using namespace std;

BlockGraph::BlockGraph(vector<shared_ptr<Block>>& commonblocks) {
    //Sort based on old locations
    sort(commonblocks.begin(), commonblocks.end(), compareOld);

    //Create adjacency list for every block
    for(auto iter = commonblocks.begin(); iter != commonblocks.end(); iter++) {
        auto insertediter = G.emplace(make_pair(*iter, vector<shared_ptr<Block>>{})).first;
        
        //All potential neighbors will be strictly after the current block
        for(auto neighboriter = iter+1; neighboriter != commonblocks.end(); neighboriter++) {
            if((*neighboriter)->oldloc > (*iter)->oldendloc() && (*neighboriter)->newloc > (*iter)->newendloc())
                insertediter->second.push_back(*neighboriter);
        }
    }
    
    vertices = commonblocks;
}

const vector<shared_ptr<Block>>& BlockGraph::getAdjacencyList(shared_ptr<Block> V) {
    return G[V];
}

const vector<shared_ptr<Block>>& BlockGraph::getAllVertices() const {
    return vertices;
}

//Create a topological ordering of graph via DFS
vector<shared_ptr<Block>> topologicalSort(BlockGraph& graph) {
    vector<shared_ptr<Block>> toporder;
    unordered_set<shared_ptr<Block>> visited;
    const vector<shared_ptr<Block>> vertices = graph.getAllVertices();
    for(shared_ptr<Block> i : vertices)
        explore(graph, i, visited, toporder);
    
    reverse(toporder.begin(), toporder.end());
    return toporder;
}

//Helper function for topologicalSort, explores all vertices connected to current
void explore(BlockGraph& graph, shared_ptr<Block> current, unordered_set<shared_ptr<Block>>& visited, vector<shared_ptr<Block>>& toporder) {
    //do not explore if already visited
    if(visited.find(current) != visited.end())
        return;
    visited.insert(current);
    vector<shared_ptr<Block>> adjacent = graph.getAdjacencyList(current);
    for(shared_ptr<Block> i : adjacent) {
        //Not visited yet
        if(visited.find(i) == visited.end())
            explore(graph, i, visited, toporder);
    }
    toporder.push_back(current);
}