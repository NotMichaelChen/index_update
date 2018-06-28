#include "distancetable.h"

#include <algorithm>
#include <iostream>

using namespace std;

//Assumes that commonblocks is already sorted by old position
vector<shared_ptr<Block>> getAdjacencyList(vector<shared_ptr<Block>>& commonblocks, size_t index) {
    vector<shared_ptr<Block>> adjacencylist;

    shared_ptr<Block>& source = commonblocks[index];
    
    //All potential neighbors will be strictly after the current block
    for(auto neighborindex = index+1; neighborindex < commonblocks.size(); neighborindex++) {
        shared_ptr<Block>& neighbor = commonblocks[neighborindex];
        if(neighbor->oldloc > source->oldendloc() && neighbor->newloc > source->newendloc())
            adjacencylist.push_back(neighbor);
    }

    return adjacencylist;
}

DistanceTable::DistanceTable(int blocklimit, std::vector<Cluster>& vertexclusters, std::vector<std::shared_ptr<Block>>& commonblocks)
    : maxsteps(blocklimit)
{
    //Initialize all vertices in graph
    for(shared_ptr<Block>& vertex : commonblocks) {
        this->initVertex(vertex);
    }

    //Sort the clusters and the common blocks
    sort(vertexclusters.begin(), vertexclusters.end(), compareCluster);
    sort(commonblocks.begin(), commonblocks.end(), compareOld);

    for(Cluster& common_vertices : vertexclusters) {
        vector<TableEntry> merged_table;

        for(shared_ptr<Block>& v : common_vertices.vertices) {
            vector<TableEntry>& from_table = tablelist.find(v)->second;

            //If neighbor's distlist is not large enough for comparing, resize it
            if(merged_table.size() < from_table.size())
                merged_table.resize(from_table.size(), TableEntry(-1, -1, nullptr, nullptr));

            //Compare each entry in prev to the entry in next+1
            for(size_t i = 0; i < from_table.size(); i++) {
                if(from_table[i].current == nullptr)
                    continue;
                
                //Take entries that beat the table's old distance
                if(from_table[i].distance > merged_table[i].distance) {
                    merged_table[i] = from_table[i];
                }
            }
        }

        //Get the adjacency list from the first vertex in the cluster
        auto iter_result = lower_bound(commonblocks.begin(), commonblocks.end(), common_vertices.vertices[0], compareOld);
        vector<shared_ptr<Block>> adjacency_list = getAdjacencyList(commonblocks, iter_result - commonblocks.begin());

        for(shared_ptr<Block>& neighbor : adjacency_list) {
            mergeIntoNext(merged_table, neighbor);
        }
    }
}

DistanceTable::DistanceTable(int blocklimit, vector<shared_ptr<Block>>& commonblocks) : maxsteps(blocklimit) {
    //Initialize all vertices in graph
    for(shared_ptr<Block>& vertex : commonblocks) {
        this->initVertex(vertex);
    }

    sort(commonblocks.begin(), commonblocks.end(), compareOld);
    
    //Fill out the dist list for each vertex
    size_t index = 0;
    for(shared_ptr<Block>& vertex : commonblocks) {
        vector<shared_ptr<Block>> adjacencylist = getAdjacencyList(commonblocks, index);
        for(shared_ptr<Block>& neighbor : adjacencylist) {
            this->mergeIntoNext(vertex, neighbor);
        }
        index++;
    }
}

DistanceTable::DistanceTable(int blocklimit, BlockGraph& graph, vector<shared_ptr<Block>>& toporder) : maxsteps(blocklimit) {
    //Initialize all vertices in graph
    //Since order doesn't matter, use toporder
    for(shared_ptr<Block> vertex : toporder) {
        this->initVertex(vertex);
    }
    
    //Fill out the dist list for each vertex
    for(shared_ptr<Block> vertex : toporder) {
        vector<shared_ptr<Block>> adjacencylist = graph.getAdjacencyList(vertex);
        for(shared_ptr<Block> neighbor : adjacencylist) {
            this->mergeIntoNext(vertex, neighbor);
        }
    }
}

vector<DistanceTable::TableEntry> DistanceTable::findAllBestPaths() {
    //Initialize our vector to fit the results
    //The actual list may be shorter than maxsteps if that graph can be traversed using
    //less than "maxsteps" hops
    vector<DistanceTable::TableEntry> bestlist;
    bestlist.resize(maxsteps, DistanceTable::TableEntry(-1, -1, nullptr, nullptr));
    
    //find the longest distance
    for(auto iter = tablelist.begin(); iter != tablelist.end(); iter++) {
        vector<DistanceTable::TableEntry> testtable = iter->second;
        for(size_t i = 0; i < testtable.size(); i++) {;
            if(testtable[i].distance > bestlist[i].distance) {
                bestlist[i] = testtable[i];
            }
        }
    }
    
    //Trim any null pairs remaining, but don't trim if the list is empty
    while(bestlist.back().current == nullptr && bestlist.size() > 0)
        bestlist.pop_back();
    
    return bestlist;
}

vector<shared_ptr<Block>> DistanceTable::findOptimalPath(int a) {
    vector<DistanceTable::TableEntry> bestlist = findAllBestPaths();
    //int refers to steps, not weight
    DistanceTable::TableEntry bestending(-1, -1, nullptr, nullptr);
    
    int prevtotalweight = 0;
    for(size_t i = 0; i < bestlist.size(); ++i) {
        int curtotalweight = bestlist[i].distance;
        if(curtotalweight == 0) break;
        if(curtotalweight - prevtotalweight < 0) break;
        
        unsigned int margin = curtotalweight - prevtotalweight;
        //ax - y > 0?
        //If there isn't enough saved common text to take the next pair, then return the current one
        if(margin < a*(i+1)) {
            break;
        }
        
        bestending = bestlist[i];
        prevtotalweight = curtotalweight;
    }
    
    if(bestending.current == nullptr && !bestlist.empty())
        bestending = bestlist.back();
    
    return tracePath(bestending);
}

vector<shared_ptr<Block>> DistanceTable::tracePath(DistanceTable::TableEntry ending) {
    vector<shared_ptr<Block>> path;
    if(ending.current == nullptr)
        return path;
    
    path.push_back(ending.current);
    DistanceTable::TableEntry candidate = ending;
    
    while(candidate.prev != nullptr) {
        path.push_back(candidate.prev);
        candidate = getPreviousEntry(candidate);
    }
    
    reverse(path.begin(), path.end());
    
    return path;
}

void DistanceTable::mergeIntoNext(shared_ptr<Block> prev, shared_ptr<Block> next) {
    int weight = next->len;

    auto& prevblock = tablelist.find(prev)->second;
    auto& nextblock = tablelist.find(next)->second;
    
    //If neighbor's distlist is not large enough for comparing, resize it
    if(nextblock.size() < prevblock.size()+1)
        nextblock.resize(prevblock.size()+1, DistanceTable::TableEntry(-1, -1, nullptr, nullptr));

    //Compare each entry in prev to the entry in next+1
    for(size_t i = 0; i < prevblock.size(); i++) {
        if(prevblock[i].current == nullptr)
            continue;
        
        if(prevblock[i].distance + weight > nextblock[i+1].distance) {
            nextblock[i+1].steps = prevblock[i].steps + 1;
            nextblock[i+1].distance = prevblock[i].distance + weight;
            nextblock[i+1].current = next;
            nextblock[i+1].prev = prev;
        }
    }

    //If the table exceeds the number of steps, remove the extra entry
    if(nextblock.size() > maxsteps)
        nextblock.pop_back();
}

void DistanceTable::mergeIntoNext(std::vector<TableEntry> merged_table, std::shared_ptr<Block> next) {
    int weight = next->len;

    auto& nextblock = tablelist.find(next)->second;
    
    //If neighbor's distlist is not large enough for comparing, resize it
    if(nextblock.size() < merged_table.size()+1)
        nextblock.resize(merged_table.size()+1, DistanceTable::TableEntry(-1, -1, nullptr, nullptr));

    //Compare each entry in prev to the entry in next+1
    for(size_t i = 0; i < merged_table.size(); i++) {
        if(merged_table[i].current == nullptr)
            continue;
        
        if(merged_table[i].distance + weight > nextblock[i+1].distance) {
            nextblock[i+1].steps = merged_table[i].steps + 1;
            nextblock[i+1].distance = merged_table[i].distance + weight;
            nextblock[i+1].current = next;
            nextblock[i+1].prev = merged_table[i].current;
        }
    }

    //If the table exceeds the number of steps, remove the extra entry
    if(nextblock.size() > maxsteps)
        nextblock.pop_back();
}

DistanceTable::TableEntry DistanceTable::getPreviousEntry(DistanceTable::TableEntry te) {
    if(te.steps < 2)
        return DistanceTable::TableEntry(-1, -1, nullptr, nullptr);
    //subtract one to offset step/index difference, subtract another one to get previous step
    return tablelist[te.prev][te.steps-2];
}

void DistanceTable::initVertex(shared_ptr<Block> V) {
    if(tablelist.find(V) == tablelist.end()) {
        vector<DistanceTable::TableEntry> tableinit;
        tableinit.push_back(DistanceTable::TableEntry(1, V->len, V, nullptr));
        //Assume that an invalid block in prev refers to the source node
        tablelist[V] = tableinit;
    }
}