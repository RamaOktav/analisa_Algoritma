#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <set>
#include <fstream>
#include <string>

// 1. Data Structures
struct Coordinate {
    int id;
    double x, y;
};

struct Edge {
    int targetNode;
    double weight;
};

struct MapGraph {
    int V;
    std::vector<Coordinate> nodes;
    std::vector<std::vector<Edge>> adjList;
};

// Helper: Calculate Euclidean Distance (Your Heuristic h(n))
double getDistance(const Coordinate& a, const Coordinate& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

// 2. The Generator Function
MapGraph generateConnectedKNNGraph(int V, int K, double mapWidth, double mapHeight) {
    MapGraph graph;
    graph.V = V;
    graph.nodes.resize(V);
    graph.adjList.resize(V);

    // Setup random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distX(0.0, mapWidth);
    std::uniform_real_distribution<> distY(0.0, mapHeight);

    // Step A: Generate random coordinates
    for (int i = 0; i < V; i++) {
        graph.nodes[i] = {i, distX(gen), distY(gen)};
    }

    std::set<std::pair<int, int>> uniqueEdges;

    // Step B: THE PURE KNN MESH (No Skeleton!)
    for (int i = 0; i < V; i++) {
        std::vector<std::pair<double, int>> distances; 
        for (int j = 0; j < V; j++) {
            if (i != j) {
                distances.push_back({getDistance(graph.nodes[i], graph.nodes[j]), j});
            }
        }

        std::sort(distances.begin(), distances.end());

        // Take the top K closest and add to our edge set ("OR" Rule)
        for (int k = 0; k < K && k < distances.size(); k++) {
            int neighborID = distances[k].second;
            uniqueEdges.insert({std::min(i, neighborID), std::max(i, neighborID)});
        }
    }

    // Step C: Build the Initial Adjacency List
    for (const auto& edgePair : uniqueEdges) {
        int u = edgePair.first;
        int v = edgePair.second;
        double weight = getDistance(graph.nodes[u], graph.nodes[v]);

        graph.adjList[u].push_back({v, weight});
        graph.adjList[v].push_back({u, weight});
    }

    // Step D: THE COMPONENT BRIDGE (Option 3 - BFS Island Connector)
    // We will repeatedly find disconnected islands and bridge them together.
    while (true) {
        std::vector<bool> visited(V, false);
        std::vector<std::vector<int>> components;

        // 1. Run BFS to find all connected continents/islands
        for (int i = 0; i < V; i++) {
            if (!visited[i]) {
                std::vector<int> currentIsland;
                std::vector<int> queue;
                
                queue.push_back(i);
                visited[i] = true;
                
                int head = 0;
                while (head < queue.size()) {
                    int curr = queue[head++];
                    currentIsland.push_back(curr);
                    for (const Edge& e : graph.adjList[curr]) {
                        if (!visited[e.targetNode]) {
                            visited[e.targetNode] = true;
                            queue.push_back(e.targetNode);
                        }
                    }
                }
                components.push_back(currentIsland);
            }
        }

        // 2. If there is only 1 component, the graph is 100% connected! We are done.
        if (components.size() == 1) {
            break; 
        }

        // 3. Otherwise, we have islands. We will build a bridge between Island 0 and Island 1.
        int bestU = -1, bestV = -1;
        double minBridgeDist = 1e9; // Infinity

        // Find the absolute shortest physical distance between the two islands
        for (int u : components[0]) {
            for (int v : components[1]) {
                double dist = getDistance(graph.nodes[u], graph.nodes[v]);
                if (dist < minBridgeDist) {
                    minBridgeDist = dist;
                    bestU = u;
                    bestV = v;
                }
            }
        }

        // 4. Build the bridge! Add the undirected edge to connect them.
        graph.adjList[bestU].push_back({bestV, minBridgeDist});
        graph.adjList[bestV].push_back({bestU, minBridgeDist});
        
        // The while loop will now repeat, run BFS again, see one fewer island, and continue until perfect.
    }

    return graph;
}

// Helper: Convert integer ID (0, 1, 26) to String Name (A1, B1, A2)
std::string getNodeName(int id) {
    char letter = 'A' + (id % 26);
    int number = (id / 26) + 1;
    return std::string(1, letter) + std::to_string(number);
}

void saveGraphToFile(const MapGraph& graph, const std::string& filename) {
    std::ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        std::cerr << "CRITICAL ERROR: Could not create file " << filename << "\n";
        return;
    }

    int totalEdges = 0;
    for (int i = 0; i < graph.V; i++) {
        totalEdges += graph.adjList[i].size();
    }
    totalEdges /= 2; 

    outFile << graph.V << " " << totalEdges << "\n";

    // Write Coordinates using Alphanumeric Names
    for (int i = 0; i < graph.V; i++) {
        outFile << getNodeName(graph.nodes[i].id) << " " 
                << graph.nodes[i].x << " " 
                << graph.nodes[i].y << "\n";
    }

    // Write Edges using Alphanumeric Names
    for (int u = 0; u < graph.V; u++) {
        for (const Edge& edge : graph.adjList[u]) {
            int v = edge.targetNode;
            if (u < v) { 
                outFile << getNodeName(u) << " " 
                        << getNodeName(v) << " " 
                        << edge.weight << "\n";
            }
        }
    }

    outFile.close();
    std::cout << "Graph securely saved to " << filename << " with Alphanumeric IDs.\n";
}

int main() {
    std::cout << "Generating Map (This might take a second for Option 3 bridging)...\n";
    
    // Generate a map with 1000 vertices, K=4, on a 500x500 grid
    MapGraph myMap = generateConnectedKNNGraph(10000, 4, 5000.0, 5000.0);
    
    int totalEdges = 0;
    for(int i = 0; i < myMap.V; i++) {
        totalEdges += myMap.adjList[i].size();
    }
    
    std::cout << "\nMap Generated Successfully!\n";
    std::cout << "Vertices: " << myMap.V << "\n";
    std::cout << "Total Undirected Edges: " << totalEdges / 2 << "\n";
    std::cout << "Average Degree: " << (double)totalEdges / myMap.V << "\n";

    std::string filename = "map_1000.txt";
    saveGraphToFile(myMap, filename);

    return 0;
}