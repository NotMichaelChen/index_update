#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <vector>
#include <set>

#include "block.h"

namespace Matcher {
    //A graph of text blocks
    class BlockGraph {
    public:
        //Initializes the graph with a vector of blocks
        BlockGraph(std::vector<std::shared_ptr<Block>>& commonblocks);
        
        //Get the adjacency list associated with V
        //Returns an empty list if not found
        std::vector<std::shared_ptr<Block>> getAdjacencyList(std::shared_ptr<Block> V);
        
        //Inserts a neighbor into V's adjacency list
        //Create a list for V if it doesn't exist in G
        void insertNeighbor(std::shared_ptr<Block> V, std::shared_ptr<Block> neighbor);
        
        //Gets the list of all vertices that appear in the graph
        const std::vector<std::shared_ptr<Block>>& getAllVertices() const;
        
    private:
        //Each Block is associated with an adjacency list of neighboring vertices
        //Edge weights aren't required since weights are defined in each vertex
        std::unordered_map<std::shared_ptr<Block>, std::vector<std::shared_ptr<Block>>> G;
        
        //List of all vertices that appear in the graph
        //Not guaranteed to be sorted in any way
        std::vector<std::shared_ptr<Block>> vertices;
    };
    
    std::vector<std::shared_ptr<Block>> topologicalSort(BlockGraph& graph);
    void explore(BlockGraph& graph, std::shared_ptr<Block> current, std::set<std::shared_ptr<Block>>& visited, std::vector<std::shared_ptr<Block>>& toporder);
}

#endif