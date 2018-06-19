#include "distancetable.h"

#include <algorithm>

using namespace std;

DistanceTable::DistanceTable(int blocklimit, BlockGraph& graph, vector<Block>& toporder) : maxsteps(blocklimit) {
    //Initialize all vertices in graph
    //Since order doesn't matter, use toporder
    for(Block vertex : toporder) {
        this->initVertex(vertex);
    }
    
    //Fill out the dist list for each vertex
    for(Block vertex : toporder) {
        vector<Block> adjacencylist = graph.getAdjacencyList(vertex);
        for(Block neighbor : adjacencylist) {
            this->mergeIntoNext(vertex, neighbor);
        }
    }
}

vector<DistanceTable::TableEntry> DistanceTable::findAllBestPaths() {
    //Initialize our vector to fit the results
    //The actual list may be shorter than maxsteps if that graph can be traversed using
    //less than "maxsteps" hops
    vector<DistanceTable::TableEntry> bestlist;
    bestlist.resize(maxsteps, DistanceTable::TableEntry(-1, -1, {}, {}));
    
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
    while(!bestlist.back().current.isValid() && bestlist.size() > 0)
        bestlist.pop_back();
    
    return bestlist;
}

vector<Block> DistanceTable::findOptimalPath(int a) {
    vector<DistanceTable::TableEntry> bestlist = findAllBestPaths();
    //int refers to steps, not weight
    DistanceTable::TableEntry bestending(-1, -1, {}, {});
    
    int prevtotalweight = 0;
    for(size_t i = 0; i < bestlist.size(); ++i) {
        int curtotalweight = bestlist[i].distance;
        if(curtotalweight == 0) break;
        if(curtotalweight - prevtotalweight < 0) break;
        
        unsigned int margin = curtotalweight - prevtotalweight;
        //ax - y > 0?
        //If there isn't enough saved common text to take the next pair, then return the current one
        if(margin < a*(i+1)) {
            //Number of steps is one greater than the index
            bestending = bestlist[i];
            break;
        }
        
        prevtotalweight = curtotalweight;
    }
    
    if(!bestending.current.isValid() && !bestlist.empty())
        bestending = bestlist.back();
    
    return tracePath(bestending);
}

vector<Block> DistanceTable::tracePath(DistanceTable::TableEntry ending) {
    vector<Block> path;
    if(!ending.current.isValid())
        return path;
    
    path.push_back(ending.current);
    DistanceTable::TableEntry candidate = ending;
    
    while(candidate.prev.isValid()) {
        path.push_back(candidate.prev);
        candidate = getPreviousEntry(candidate);
    }
    
    reverse(path.begin(), path.end());
    
    return path;
}

void DistanceTable::mergeIntoNext(Block prev, Block next) {
    int weight = next.run.size();

    auto& prevblock = tablelist.find(prev)->second;
    auto& nextblock = tablelist.find(next)->second;
    
    //If neighbor's distlist is not large enough for comparing, resize it
    if(nextblock.size() < prevblock.size()+1)
        nextblock.resize(prevblock.size()+1, DistanceTable::TableEntry(-1, -1, {}, {}));

    //Compare each entry in prev to the entry in next+1
    for(size_t i = 0; i < prevblock.size(); i++) {
        if(!prevblock[i].current.isValid())
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

DistanceTable::TableEntry DistanceTable::getPreviousEntry(DistanceTable::TableEntry te) {
    if(te.steps < 2)
        return DistanceTable::TableEntry(-1, -1, {}, {});
    //subtract one to offset step/index difference, subtract another one to get previous step
    return tablelist[te.prev][te.steps-2];
}

void DistanceTable::initVertex(Block V) {
    if(tablelist.find(V) == tablelist.end()) {
        vector<DistanceTable::TableEntry> tableinit;
        tableinit.push_back(DistanceTable::TableEntry(1, V.run.size(), V, {}));
        //Assume that an invalid block in prev refers to the source node
        tablelist[V] = tableinit;
    }
}