#include "graph.h"

#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include <iostream>

#include "block.h"

using namespace std;

//Constructs a graph from a list of blocks
//Each vertex is a block, and each edge represents a valid vertex to take
map<Block*, vector<Block*>> makeGraph(vector<Block*>& commonblocks) {
    //Sort based on old locations
    sort(commonblocks.begin(), commonblocks.end(), compareOld);
    
    map<Block*, vector<Block*>> graph;
    
    //Create adjacency list for every block
    for(int i = 0; i < commonblocks.size(); i++) {
        Block* current = commonblocks[i];
        
        vector<Block*> edges;
        
        //All potential neighbors will be strictly after the current block
        for(size_t neighborindex = i+1; neighborindex < commonblocks.size(); neighborindex++) {
            Block* neighbor = commonblocks[neighborindex];
            //Must be >= not > since oldloc+length extends 1 position beyond the run
            if(neighbor->oldloc >= current->oldloc + (current->run).size() &&
                neighbor->newloc >= current->newloc + (current->run).size())
            {
                edges.push_back(neighbor);
            }
        }
        
        graph[current] = edges;
    }
    
    //Insert "source" node that is connected to everything
    graph[nullptr] = commonblocks;
    
    return graph;
}

//Create a topological ordering of G via DFS
vector<Block*> topologicalSort(map<Block*, vector<Block*>>& G) {
    vector<Block*> toporder;
    set<Block*> visited;
    explore(G, nullptr, visited, toporder);
    reverse(toporder.begin(), toporder.end());
    return toporder;
}

//Helper function for topologicalSort, explores all vertices connected to current
void explore(map<Block*, vector<Block*>>& G, Block* current, set<Block*>& visited, vector<Block*>& toporder) {
    visited.insert(current);
    vector<Block*> adjacent = G[current];
    for(Block* i : adjacent) {
        //Not visited yet
        if(visited.find(i) == visited.end())
            explore(G, i, visited, toporder);
    }
    toporder.push_back(current);
}

//Creates a table of the best distances up to a certain block using k steps
//block -> {distance, prev}(k=1) , {distance, prev}(k=2), ...
map<Block*, vector<pair<int, Block*>>> getDistanceTable(int steps, map<Block*, vector<Block*>>& G, vector<Block*>& toporder) {
    //Table of best distances ending at a given block using k steps
    //Also stores previous block to find the optimal path
    map<Block*, vector<pair<int, Block*>>> disttable;
    //Insert "source" into the dist table
    vector<pair<int, Block*>> dist;
    dist.emplace_back(0, nullptr);
    disttable[nullptr] = dist;
    
    //Fill out the dist table for each vertex
    for(Block* vertex : toporder) {
        for(Block* neighbor : G[vertex]) {
            int weight = neighbor->run.size();
            //initialize vertex if not found
            if(disttable.find(neighbor) == disttable.end()) {
                vector<pair<int, Block*>> initdist;
                //No need to worry about distance; since S points to everything
                //all nodes will have their weight at k=1
                initdist.emplace_back(weight, vertex);
                disttable[neighbor] = initdist;
            }
            else {
                //If neighbor's disttable is not large enough for comparing, resize it
                if(disttable[neighbor].size() < disttable[vertex].size()+1)
                    disttable[neighbor].resize(disttable[vertex].size()+1, make_pair(0, nullptr));
                
                for(size_t i = 0; i < disttable[vertex].size(); i++) {
                    if(disttable[vertex][i].first == 0)
                        continue;
                    
                    if(disttable[vertex][i].first + weight > disttable[neighbor][i+1].first)
                    {
                        disttable[neighbor][i+1].first = disttable[vertex][i].first + weight;
                        disttable[neighbor][i+1].second = vertex;
                    }
                }
                
                if(disttable[neighbor].size() > steps)
                    disttable[neighbor].pop_back();
            }
        }
    }
    
    return disttable;
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