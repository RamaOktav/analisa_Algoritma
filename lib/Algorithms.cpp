#include "Algorithms.h"
#include "FileIO.h" // Needed to call appendToCSV
#include <queue>
#include <iostream>
#include <algorithm>
#include <limits>

struct SearchNode {
    int id;
    double h_cost; 
    bool operator>(const SearchNode& other) const { return h_cost > other.h_cost; }
};

struct AStarNode {
    int id;
    double f_cost;
    bool operator>(const AStarNode& other) const { return f_cost > other.f_cost; }
};

// --- GREEDY IMPLEMENTATION ---
void greedySearchCore(const MapGraph& graph, int start, int target, std::vector<int>& finalPath, double& totalPathDistance, long long& nodesVisited) {
    std::priority_queue<SearchNode, std::vector<SearchNode>, std::greater<SearchNode>> pq;
    std::vector<bool> visited(graph.V, false);
    std::vector<int> cameFrom(graph.V, -1); 
    nodesVisited = 0;
    totalPathDistance = -1.0; 
    finalPath.clear();
    
    pq.push({start, getDistance(graph.nodes[start], graph.nodes[target])});
    visited[start] = true;
    bool pathFound = false;

    while (!pq.empty()) {
        int current = pq.top().id;
        pq.pop();
        nodesVisited++;
        if (current == target) { pathFound = true; break; }

        for (const Edge& edge : graph.adjList[current]) {
            int neighbor = edge.targetNode;
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                cameFrom[neighbor] = current; 
                pq.push({neighbor, getDistance(graph.nodes[neighbor], graph.nodes[target])});
            }
        }
    }

    if (pathFound) {
        int curr = target;
        while (curr != -1) {
            finalPath.push_back(curr);
            curr = cameFrom[curr];
        }
        std::reverse(finalPath.begin(), finalPath.end());
        totalPathDistance = 0;
        for (size_t i = 0; i < finalPath.size() - 1; i++) {
            int u = finalPath[i];
            int v = finalPath[i+1];
            for (const Edge& edge : graph.adjList[u]) {
                if (edge.targetNode == v) { totalPathDistance += edge.weight; break; }
            }
        }
    }
}

void runGreedyAndSave(const MapGraph& graph, int start, int target, const std::string& csvFilename) {
    std::cout << "  -> Running Greedy Best-First Search...\n";
    std::vector<int> finalPath;
    double totalPathDistance = 0.0;
    long long nodesVisited = 0;
    greedySearchCore(graph, start, target, finalPath, totalPathDistance, nodesVisited);

    if (finalPath.empty() || totalPathDistance < 0) {
        std::cout << "     [RESULT] Path Not Found.\n";
        appendToCSV(csvFilename, "Greedy", graph.V, start, target, nodesVisited, -1.0);
    } else {
        std::cout << "     [SUCCESS] Greedy Path Found!\n";
        appendToCSV(csvFilename, "Greedy", graph.V, start, target, nodesVisited, totalPathDistance);
    }
}

// --- EXHAUSTIVE IMPLEMENTATION ---
void exhaustiveSearchDFS(const MapGraph& graph, int current, int target, std::vector<bool>& visited, std::vector<int>& currentPath, double currentDist, std::vector<int>& bestPath, double& bestDist, long long& nodesVisited, bool& aborted) {
    nodesVisited++;
    if (nodesVisited > 5000000) { aborted = true; return; }
    if (aborted || currentDist >= bestDist) return; 

    if (current == target) {
        if (currentDist < bestDist) { bestDist = currentDist; bestPath = currentPath; }
        return;
    }

    for (const Edge& edge : graph.adjList[current]) {
        int neighbor = edge.targetNode;
        if (!visited[neighbor]) {
            visited[neighbor] = true;
            currentPath.push_back(neighbor);
            exhaustiveSearchDFS(graph, neighbor, target, visited, currentPath, currentDist + edge.weight, bestPath, bestDist, nodesVisited, aborted);
            currentPath.pop_back();
            visited[neighbor] = false;
        }
    }
}

void runExhaustiveAndSave(const MapGraph& graph, int start, int target, const std::string& csvFilename) {
    std::cout << "  -> Running Exhaustive Search (Brute Force)...\n";
    std::vector<bool> visited(graph.V, false);
    std::vector<int> currentPath;
    std::vector<int> bestPath;
    double bestDist = std::numeric_limits<double>::infinity();
    long long nodesVisited = 0;
    bool aborted = false;

    visited[start] = true;
    currentPath.push_back(start);
    exhaustiveSearchDFS(graph, start, target, visited, currentPath, 0.0, bestPath, bestDist, nodesVisited, aborted);

    if (aborted || bestDist == std::numeric_limits<double>::infinity()) {
        std::cout << "     [RESULT] Aborted or No Path.\n";
        appendToCSV(csvFilename, "Exhaustive", graph.V, start, target, nodesVisited, -1.0);
    } else {
        std::cout << "     [SUCCESS] Shortest path perfectly guaranteed!\n";
        appendToCSV(csvFilename, "Exhaustive", graph.V, start, target, nodesVisited, bestDist);
    }
}

// --- A* IMPLEMENTATION ---
void aStarSearchCore(const MapGraph& graph, int start, int target, std::vector<int>& finalPath, double& totalPathDistance, long long& nodesVisited) {
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> pq;
    std::vector<double> gScore(graph.V, std::numeric_limits<double>::infinity());
    std::vector<int> cameFrom(graph.V, -1); 
    nodesVisited = 0;
    totalPathDistance = -1.0; 
    finalPath.clear();
    
    gScore[start] = 0.0;
    pq.push({start, getDistance(graph.nodes[start], graph.nodes[target])});
    bool pathFound = false;

    while (!pq.empty()) {
        int current = pq.top().id;
        pq.pop();
        nodesVisited++;

        if (current == target) { pathFound = true; break; }

        for (const Edge& edge : graph.adjList[current]) {
            int neighbor = edge.targetNode;
            double tentative_gScore = gScore[current] + edge.weight;
            
            if (tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                double h = getDistance(graph.nodes[neighbor], graph.nodes[target]);
                pq.push({neighbor, tentative_gScore + h});
            }
        }
    }

    if (pathFound) {
        int curr = target;
        while (curr != -1) { finalPath.push_back(curr); curr = cameFrom[curr]; }
        std::reverse(finalPath.begin(), finalPath.end());
        totalPathDistance = gScore[target];
    }
}

void runAStarAndSave(const MapGraph& graph, int start, int target, const std::string& csvFilename) {
    std::cout << "  -> Running A* Search...\n";
    std::vector<int> finalPath;
    double totalPathDistance = 0.0;
    long long nodesVisited = 0;
    aStarSearchCore(graph, start, target, finalPath, totalPathDistance, nodesVisited);

    if (finalPath.empty() || totalPathDistance < 0) {
        std::cout << "     [RESULT] Path Not Found.\n";
        appendToCSV(csvFilename, "A_Star", graph.V, start, target, nodesVisited, -1.0);
    } else {
        std::cout << "     [SUCCESS] A* Path Found!\n";
        appendToCSV(csvFilename, "A_Star", graph.V, start, target, nodesVisited, totalPathDistance);
    }
}