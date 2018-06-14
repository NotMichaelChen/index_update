#include "graph.h"

#include <algorithm>

using namespace std;

namespace Matcher {
    BlockGraph::BlockGraph(vector<Block>& commonblocks) {
        //Sort based on old locations
        sort(commonblocks.begin(), commonblocks.end(), compareOld);
        
        //Create adjacency list for every block
        for(size_t i = 0; i < commonblocks.size(); i++) {
            Block current = commonblocks[i];
            G[current] = vector<Block>();
            
            //All potential neighbors will be strictly after the current block
            for(size_t neighborindex = i+1; neighborindex < commonblocks.size(); neighborindex++) {
                Block neighbor = commonblocks[neighborindex];
                if(neighbor.oldloc > current.oldendloc() && neighbor.newloc > current.newendloc())
                    this->insertNeighbor(current, neighbor);
            }
        }
        
        int edges = 0;
        for(auto iter = G.begin(); iter != G.end(); iter++) {
            edges += iter->second.size();
        }
        
        vertices = commonblocks;
    }
    
    vector<Block> BlockGraph::getAdjacencyList(Block V) {
        //if V is found
        if(G.find(V) != G.end()) {
            return G[V];
        }
        else {
            return vector<Block>();
        }
    }                
    
    void BlockGraph::insertNeighbor(Block V, Block neighbor) {
        //if V is found
        if(G.find(V) != G.end()) {
            G[V].push_back(neighbor);
        }
        else {
            vector<Block> edges;
            edges.push_back(neighbor);
            G[V] = edges;
            vertices.push_back(V);
        }
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
}