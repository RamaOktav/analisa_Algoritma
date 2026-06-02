#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <set>
#include <fstream>
#include <string>
#include <filesystem>
#include <random>

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

double randFloat(double start, double end)
{
  static std::random_device rd;
  static std::mt19937 mt(rd());
  std::uniform_real_distribution<> dist(start, end);
  return dist(mt);
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
        double baseDistance = getDistance(graph.nodes[u], graph.nodes[v]);

        // 2. Simulate winding roads (Multiply by 1.0 - 2.0)
        double roadFriction = randFloat(1.0, 2.0);
        double finalWeight = baseDistance * roadFriction;

        graph.adjList[u].push_back({v, finalWeight});
        graph.adjList[v].push_back({u, finalWeight});
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

void saveGraphToFile(const MapGraph& graph, const std::string& filename) {
    std::filesystem::path filePath(filename);
    std::filesystem::path dir = filePath.parent_path();
    
    // If there is a folder in the path, and it doesn't exist, create it!
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
        std::cout << "Created missing directory: " << dir << "\n";
    }

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

    // Write Coordinates using standard integer IDs
    for (int i = 0; i < graph.V; i++) {
        outFile << graph.nodes[i].id << " " 
                << graph.nodes[i].x << " " 
                << graph.nodes[i].y << "\n";
    }

    // Write Edges using standard integer IDs
    for (int u = 0; u < graph.V; u++) {
        for (const Edge& edge : graph.adjList[u]) {
            int v = edge.targetNode;
            if (u < v) { 
                outFile << u << " " 
                        << v << " " 
                        << edge.weight << "\n";
            }
        }
    }

    outFile.close();
    std::cout << "Graph securely saved to " << filename << " with Alphanumeric IDs.\n";
}


std::vector<int> generateVertexSizes() {
    std::vector<int> vertices;
    int v = 16;
    while(v <= 10000){
        vertices.push_back(v);
        if(v < 500) v = static_cast<int>(v * 1.2);
        else v *= 1.3;
    }
    return vertices;
}

double XYsize(int vertices){
    // We want a density of roughly 1 city per 50x50 area unit
    // Base: 100 vertices -> 500x500 area
    double baseSize = 500.0;
    double baseVertices = 100.0;
    
    // The side length grows with the square root of the number of vertices
    // to maintain a consistent density
    return baseSize * std::sqrt((double)vertices / baseVertices);
}

int main() {
    std::ios_base::sync_with_stdio(false); 
    std::cin.tie(NULL);

    std::cout << "Generating Map (This might take a second for Option 3 bridging)...\n";
    // 0 = Debug (Fast, small samples)
    // 1 = Full Stress Test (Aggressive, multi-stage)
    const int BENCHMARK_MODE = 1; 

    std::vector<int> mapSizes;

    if (BENCHMARK_MODE == 0) {
        mapSizes = {15, 100, 500, 1000};
    } else {
        mapSizes = generateVertexSizes();
    }

    int K = 3;

    for(int size : mapSizes){
        std::cout << "Processing V=" << size << "... ";
        auto start = std::chrono::high_resolution_clock::now();
        double side = XYsize(size);
        double xSize = side;
        double ySize = side;

        MapGraph myMap = generateConnectedKNNGraph(size, K, xSize, ySize);
        
        int totalEdges = 0;
        for(int i = 0; i < myMap.V; i++) {
            totalEdges += myMap.adjList[i].size();
        }
        
        std::cout << "\nMap Generated Successfully!\n";
        std::cout << "Vertices: " << myMap.V << "\n";
        std::cout << "Total Undirected Edges: " << totalEdges / 2 << "\n";
        std::cout << "Average Degree: " << (double)totalEdges / myMap.V << "\n";
    
        std::string filename = "data/map_" + std::to_string(size) + ".txt";
        saveGraphToFile(myMap, filename);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "Done in " << diff.count() << "s\n";
    }

    return 0;
}