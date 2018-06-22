#ifndef DISTANCETABLE_H
#define DISTANCETABLE_H

#include <unordered_map>
#include <vector>

#include "graph.h"
#include "block.h"

//Table that holds distance info when traversing a graph
//**All references to steps are assuming starting from some source node S that connects to all other nodes
//Thus a step=0 is impossible
class DistanceTable {
public:
    struct TableEntry {
        TableEntry(int s, int d, std::shared_ptr<Block> c, std::shared_ptr<Block> p)
        : steps(s), distance(d), current(c), prev(p) {}
        
        int steps;
        int distance;
        std::shared_ptr<Block> current;
        std::shared_ptr<Block> prev;
    };
    
    DistanceTable(int blocklimit, BlockGraph& graph, std::vector<std::shared_ptr<Block>>& toporder);
    
    
    //Finds an optimal path through the graph that balances block count vs common text
    //The parameter is the constant of the cost function, which takes the form ax+y
    //Where x is the amount of blocks taken, and y is the marginal amount of common text
    //Thus a represents the cost of taking one block of text
    //NOTE: this pair represents <steps, block> instead of <weight, block>
    //weight can be obtained elsewhere
    std::vector<std::shared_ptr<Block>> findOptimalPath(int a);
    
    //Fills a vector with a path that ends at ending
    //path is in order of graph traversal
    //ending is included in the path as the last entry
    std::vector<std::shared_ptr<Block>> tracePath(TableEntry ending);
    
private:
    //Initialize a vertex in the table
    //The vertex gets one entry in its distance list; (V.weight, nullptr)
    //This is because we can access any vertex in the graph in one step
    void initVertex(std::shared_ptr<Block> V);
    //Merge the list from prev into the list of next, where next is a neighbor of prev
    void mergeIntoNext(std::shared_ptr<Block> prev, std::shared_ptr<Block> next);
    //Gets the previous table entry given a current table entry
    TableEntry getPreviousEntry(TableEntry te);
    //Gets a list of every best path in the graph
    //index of vector refers to number of steps through graph (see assumption below)
    //Each entry is a pair of totalweight, endingblock.
    std::vector<TableEntry> findAllBestPaths();
    
    //Each block is associated with a table describing its distance from S
    //Each table is a list of distance and previous_block pairs, with the position
    //in the vector denoting the number of hops it takes from S.
    //**We assume that index=0 means 1 hop from S
    std::unordered_map<std::shared_ptr<Block>, std::vector<TableEntry>> tablelist;
    //The maximum number of steps we're allowed to take through the graph
    size_t maxsteps;
};

#endif