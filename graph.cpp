#include "graph.h"

#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include <iostream>

#include "block.h"
#include "distancetable.h"

using namespace std;

BlockGraph::BlockGraph(vector<Block*>& commonblocks) {
    //Sort based on old locations
    sort(commonblocks.begin(), commonblocks.end(), compareOld);
    
    //Create adjacency list for every block
    for(int i = 0; i < commonblocks.size(); i++) {
        Block* current = commonblocks[i];
        
        //All potential neighbors will be strictly after the current block
        for(size_t neighborindex = i+1; neighborindex < commonblocks.size(); neighborindex++) {
            Block* neighbor = commonblocks[neighborindex];
            //Must be >= not > since oldloc+length extends 1 position beyond the run
            if(neighbor->oldloc >= current->oldloc + (current->run).size() &&
                neighbor->newloc >= current->newloc + (current->run).size())
            {
                this->insertNeighbor(current, neighbor);
            }
        }
    }
    
    vertices = commonblocks;
}

vector<Block*> BlockGraph::getAdjacencyList(Block* V) {
    //if V is found
    if(G.find(V) != G.end()) {
        return G[V];
    }
    else {
        return vector<Block*>();
    }
}                

void BlockGraph::insertNeighbor(Block* V, Block* neighbor) {
    //if V is found
    if(G.find(V) != G.end()) {
        G[V].push_back(neighbor);
    }
    else {
        vector<Block*> edges;
        edges.push_back(neighbor);
        G[V] = edges;
        vertices.push_back(V);
    }
}

const vector<Block*>& BlockGraph::getAllVertices() const {
    return vertices;
}

//Create a topological ordering of graph via DFS
vector<Block*> topologicalSort(BlockGraph& graph) {
    vector<Block*> toporder;
    set<Block*> visited;
    const vector<Block*> vertices = graph.getAllVertices();
    for(Block* i : vertices)
        explore(graph, i, visited, toporder);
    
    reverse(toporder.begin(), toporder.end());
    return toporder;
}

//Helper function for topologicalSort, explores all vertices connected to current
void explore(BlockGraph& graph, Block* current, set<Block*>& visited, vector<Block*>& toporder) {
    //do not explore if already visited
    if(visited.find(current) != visited.end())
        return;
    visited.insert(current);
    vector<Block*> adjacent = graph.getAdjacencyList(current);
    for(Block* i : adjacent) {
        //Not visited yet
        if(visited.find(i) == visited.end())
            explore(graph, i, visited, toporder);
    }
    toporder.push_back(current);
}

//Finds the best (longest) path through the graph
vector<Block*> findBestPath(map<Block*, vector<pair<int, Block*>>>& disttable) {
    int greatest = 0;
    int steps = 0;
    Block* ending = nullptr;
    //find the longest distance
    for(auto iter = disttable.begin(); iter != disttable.end(); iter++) {
        if(iter->first != nullptr)
            cout << (iter->first)->oldloc << "-" << (iter->first)->newloc << endl;
        else
            cout << "S" << endl;
        
        vector<pair<int, Block*>> table = iter->second;
        for(size_t i = 0; i < table.size(); i++) {
            if(table[i].second != nullptr)
                cout << "\t" << table[i].first << " " << (table[i].second)->oldloc << "-" << (table[i].second)->newloc << endl;
            if(table[i].first > greatest) {
                ending = table[i].second;
                greatest = table[i].first;
                steps = i;
            }
        }
    }
    
    //find the path leading to the longest distance
    vector<Block*> path;
    path.push_back(ending);
    steps--;
    Block* next = ending;
    while(steps > 0) {
        next = disttable[next][steps].second;
        path.push_back(next);
        steps--;
    }
    
    return path;
}