#ifndef DISTANCETABLE_H
#define DISTANCETABLE_H

#include <map>
#include <vector>

#include "graph.h"
#include "block.h"

//Table that holds distance info when traversing a graph
class DistanceTable {
public:
    DistanceTable(int blocklimit, BlockGraph& graph, std::vector<Block*>& toporder);
    
    //Gets a list of every best path in the graph
    std::vector<std::pair<int, Block*>> findAllBestPaths();
    
    //Fills a vector with a path that ends at ending
    //ending is included in the path as the first entry
    std::vector<Block*> tracePath(Block* ending);
    
private:
    //Initialize a vertex in the table
    //The vertex gets one entry in its distance list; (V.weight, nullptr)
    //This is because we can access any vertex in the graph in one step
    void initVertex(Block* V);
    //Merge the list from prev into the list of next, where next is a neighbor of prev
    void mergeIntoNext(Block* prev, Block* next);
    
    //Each block is associated with a table describing its distance from S
    //Each table is a list of distance and previous_block pairs, with the position
    //in the vector denoting the number of hops it takes from S.
    //**We assume that index=0 means 1 hop from S
    std::map<Block*, std::vector<std::pair<int, Block*>>> table;
    //The maximum number of steps we're allowed to take through the graph
    int maxsteps;
};

#endif