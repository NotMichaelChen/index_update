#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <vector>
#include <unordered_set>

#include "block.h"

namespace Matcher {
    //A graph of text blocks
    class BlockGraph {
    public:
        //Initializes the graph with a vector of blocks
        BlockGraph(std::vector<Block>& commonblocks);
        
        //Get the adjacency list associated with V
        //* Assumes that V is a valid vertice inside of the graph *
        const std::vector<Block>& getAdjacencyList(Block V);
        
        //Gets the list of all vertices that appear in the graph
        const std::vector<Block>& getAllVertices() const;
        
    private:
        //Each Block is associated with an adjacency list of neighboring vertices
        //Edge weights aren't required since weights are defined in each vertex
        std::unordered_map<Block, std::vector<Block>> G;
        
        //List of all vertices that appear in the graph
        //Not guaranteed to be sorted in any way
        std::vector<Block> vertices;
    };
    
    std::vector<Block> topologicalSort(BlockGraph& graph);
    void explore(BlockGraph& graph, Block current, std::unordered_set<Block>& visited, std::vector<Block>& toporder);
}

#endif