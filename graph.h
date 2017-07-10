#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <vector>
#include <set>

#include "block.h"

//A graph of text blocks
class BlockGraph {
public:
    //Initializes the graph with a vector of blocks
    BlockGraph(std::vector<Block*>& commonblocks);
    
    //Get the adjacency list associated with V
    //Returns an empty list if not found
    std::vector<Block*> getAdjacencyList(Block* V);
    
    //Inserts a neighbor into V's adjacency list
    //Create a list for V if it doesn't exist in G
    void insertNeighbor(Block* V, Block* neighbor);
    
    //Gets the list of all vertices that appear in the graph
    const std::vector<Block*>& getAllVertices() const;
    
private:
    //Each Block is associated with an adjacency list of neighboring vertices
    //Edge weights aren't required since weights are defined in each vertex
    std::map<Block*, std::vector<Block*>> G;
    
    //List of all vertices that appear in the graph
    //Not guaranteed to be sorted in any way
    std::vector<Block*> vertices;
};

std::vector<Block*> topologicalSort(const BlockGraph& graph);
void explore(const BlockGraph& graph, Block* current, std::set<Block*>& visited, std::vector<Block*>& toporder);
std::map<Block*, std::vector<std::pair<int, Block*>>> getDistanceTable(int steps, std::map<Block*, std::vector<Block*>>& G, std::vector<Block*>& toporder);
std::vector<Block*> findBestPath(std::map<Block*, std::vector<std::pair<int, Block*>>>& disttable);

#endif