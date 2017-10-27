#include "graph.h"

#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include <iostream>

#include "block.h"
#include "distancetable.h"

using namespace std;

namespace Matcher {
    BlockGraph::BlockGraph(vector<shared_ptr<Block>>& commonblocks) {
        //Sort based on old locations
        sort(commonblocks.begin(), commonblocks.end(), compareOld);
        
        //Create adjacency list for every block
        for(size_t i = 0; i < commonblocks.size(); i++) {
            shared_ptr<Block> current = commonblocks[i];
            G[current] = vector<shared_ptr<Block>>();
            
            //All potential neighbors will be strictly after the current block
            for(size_t neighborindex = i+1; neighborindex < commonblocks.size(); neighborindex++) {
                shared_ptr<Block> neighbor = commonblocks[neighborindex];
                if(neighbor->oldloc > current->oldendloc() && neighbor->newloc > current->newendloc())
                    this->insertNeighbor(current, neighbor);
            }
        }
        
        vertices = commonblocks;
    }
    
    vector<shared_ptr<Block>> BlockGraph::getAdjacencyList(shared_ptr<Block> V) {
        //if V is found
        if(G.find(V) != G.end()) {
            return G[V];
        }
        else {
            return vector<shared_ptr<Block>>();
        }
    }                
    
    void BlockGraph::insertNeighbor(shared_ptr<Block> V, shared_ptr<Block> neighbor) {
        //if V is found
        if(G.find(V) != G.end()) {
            G[V].push_back(neighbor);
        }
        else {
            vector<shared_ptr<Block>> edges;
            edges.push_back(neighbor);
            G[V] = edges;
            vertices.push_back(V);
        }
    }
    
    const vector<shared_ptr<Block>>& BlockGraph::getAllVertices() const {
        return vertices;
    }
    
    //Create a topological ordering of graph via DFS
    vector<shared_ptr<Block>> topologicalSort(BlockGraph& graph) {
        vector<shared_ptr<Block>> toporder;
        set<shared_ptr<Block>> visited;
        const vector<shared_ptr<Block>> vertices = graph.getAllVertices();
        for(shared_ptr<Block> i : vertices)
            explore(graph, i, visited, toporder);
        
        reverse(toporder.begin(), toporder.end());
        return toporder;
    }
    
    //Helper function for topologicalSort, explores all vertices connected to current
    void explore(BlockGraph& graph, shared_ptr<Block> current, set<shared_ptr<Block>>& visited, vector<shared_ptr<Block>>& toporder) {
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
}