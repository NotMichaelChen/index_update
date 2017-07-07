#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <vector>
#include <set>

#include "block.h"

std::map<Block*, std::vector<Block*>> makeGraph(std::vector<Block*>& commonblocks);
std::vector<Block*> topologicalSort(std::map<Block*, std::vector<Block*>>& G);
void explore(std::map<Block*, std::vector<Block*>>& G, Block* current, std::set<Block*>& visited, std::vector<Block*>& toporder);
int longestPath(int steps, std::map<Block*, std::vector<Block*>>& G, std::vector<Block*>& toporder);

#endif