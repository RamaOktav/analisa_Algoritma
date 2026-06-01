#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <queue>
#include <cmath>
#include <algorithm>
#include <random>
#include <limits>

// 1. Data Structures (Notice everything is strictly INT internally)
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

struct SearchNode {
    int id;
    double h_cost; 

    bool operator>(const SearchNode& other) const {
        return h_cost > other.h_cost; 
    }
};

// 3. The Forward Translator (Int 26 -> String "A2", just for pretty printing)
std::string getNodeName(int id) {
    char letter = 'A' + (id % 26);
    int number = (id / 26) + 1;
    return std::string(1, letter) + std::to_string(number);
}

// 4. The Importer Function
MapGraph loadGraphFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    MapGraph graph;

    if (!inFile.is_open()) {
        std::cerr << "CRITICAL ERROR: Could not open " << filename << "\n";
        return graph;
    }

    int totalEdges;
    inFile >> graph.V >> totalEdges;

    graph.nodes.resize(graph.V);
    graph.adjList.resize(graph.V);

    for (int i = 0; i < graph.V; i++) {
        int id;
        double x, y;
        inFile >> id >> x >> y;
        
        graph.nodes[id] = {id, x, y};
    }

    // Read Edges (Strictly Integers and Doubles now)
    for (int i = 0; i < totalEdges; i++) {
        int u, v; // Changed from std::string
        double weight;
        inFile >> u >> v >> weight;

        graph.adjList[u].push_back({v, weight});
        graph.adjList[v].push_back({u, weight});
    }

    inFile.close();
    return graph;
}

// 1. Run this ONCE in main() before your testing loop starts
void initializeCSV(const std::string& filename) {
    std::ofstream outFile(filename); // Normal mode (Overwrites old data)
    if (outFile.is_open()) {
        outFile << "Algorithm,Vertices,StartNode,TargetNode,NodesVisited,TotalDistance\n";
        outFile.close();
    }
}

// 2. The Exporter (Used by Greedy, Exhaustive, and A*)
void appendToCSV(const std::string& filename, const std::string& algo, int V, int start, int target, int nodesVisited, double distance) {
    std::ofstream outFile;
    
    // std::ios::app stands for APPEND MODE. 
    // It guarantees we add to the bottom of the CSV instead of deleting it!
    outFile.open(filename, std::ios::app); 
    
    if (outFile.is_open()) {
        outFile << algo << "," 
                << V << "," 
                << start << "," 
                << target << "," 
                << nodesVisited << "," 
                << distance << "\n";
        outFile.close();
    } else {
        std::cerr << "CRITICAL ERROR: Could not open " << filename << " to append data.\n";
    }
}

double getDistance(const Coordinate& a, const Coordinate& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

// THE ENGINE: Pure Math and Pathfinding
void greedySearchCore(const MapGraph& graph, int start, int target, 
                      std::vector<int>& finalPath, double& totalPathDistance, 
                      long long& nodesVisited) {
    
    std::priority_queue<SearchNode, std::vector<SearchNode>, std::greater<SearchNode>> pq;
    std::vector<bool> visited(graph.V, false);
    std::vector<int> cameFrom(graph.V, -1); 
    
    nodesVisited = 0;
    totalPathDistance = -1.0; // Default to -1.0 to indicate failure
    finalPath.clear();
    
    pq.push({start, getDistance(graph.nodes[start], graph.nodes[target])});
    visited[start] = true;

    bool pathFound = false;

    // The Core Loop
    while (!pq.empty()) {
        int current = pq.top().id;
        pq.pop();
        
        nodesVisited++;

        // Target Reached!
        if (current == target) {
            pathFound = true;
            break;
        }

        // Evaluate Neighbors
        for (const Edge& edge : graph.adjList[current]) {
            int neighbor = edge.targetNode;
            
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                cameFrom[neighbor] = current; 
                
                double h = getDistance(graph.nodes[neighbor], graph.nodes[target]);
                pq.push({neighbor, h});
            }
        }
    }

    // Path Reconstruction
    if (pathFound) {
        int curr = target;
        while (curr != -1) {
            finalPath.push_back(curr);
            curr = cameFrom[curr];
        }
        std::reverse(finalPath.begin(), finalPath.end());

        // CORRECTED: Calculate the TRUE path distance using the Winding Road weights
        totalPathDistance = 0;
        for (size_t i = 0; i < finalPath.size() - 1; i++) {
            int u = finalPath[i];
            int v = finalPath[i+1];
            
            // Search the adjacency list to find the exact road between u and v
            for (const Edge& edge : graph.adjList[u]) {
                if (edge.targetNode == v) {
                    totalPathDistance += edge.weight; // Use the penalized Winding Road weight!
                    break;
                }
            }
        }
    }
}

// THE WRAPPER: Handles Console Output and CSV Export
void runGreedyAndSave(const MapGraph& graph, int start, int target, const std::string& csvFilename) {
    std::cout << "  -> Running Greedy Best-First Search...\n";
    
    std::vector<int> finalPath;
    double totalPathDistance = 0.0;
    long long nodesVisited = 0;

    // 1. Launch the Engine
    greedySearchCore(graph, start, target, finalPath, totalPathDistance, nodesVisited);

    // 2. Handle the Results
    if (finalPath.empty() || totalPathDistance < 0) {
        std::cout << "     [RESULT] Path Not Found.\n";
        appendToCSV(csvFilename, "Greedy", graph.V, start, target, nodesVisited, -1.0);
    } 
    else {
        std::cout << "     [SUCCESS] Greedy Path Found!\n";
        std::cout << "     Nodes Visited: " << nodesVisited << "\n";
        std::cout << "     Total Distance: " << totalPathDistance << "\n";
        
        // Save clean data to the CSV for Python analytics
        appendToCSV(csvFilename, "Greedy", graph.V, start, target, nodesVisited, totalPathDistance);
    }
}

// The Recursive Engine
void exhaustiveSearchDFS(const MapGraph& graph, int current, int target, 
                         std::vector<bool>& visited, std::vector<int>& currentPath, 
                         double currentDist, std::vector<int>& bestPath, double& bestDist, 
                         long long& nodesVisited, bool& aborted) {
    
    // 1. THE KILL SWITCH: Abort if it takes too long (5 Million nodes)
    nodesVisited++;
    if (nodesVisited > 5000000) {
        aborted = true;
        return;
    }
    if (aborted) return; // Fast exit if kill switch was pulled deeper in the recursion

    // 2. PRUNING: If our current path is already worse than the best we found, stop searching it!
    if (currentDist >= bestDist) {
        return;
    }

    // 3. BASE CASE: We hit the target!
    if (current == target) {
        if (currentDist < bestDist) {
            bestDist = currentDist;
            bestPath = currentPath; // Copy the array
        }
        return;
    }

    // 4. THE EXPANSION & BACKTRACKING
    for (const Edge& edge : graph.adjList[current]) {
        int neighbor = edge.targetNode;
        
        if (!visited[neighbor]) {
            // Step Forward (Plunge deeper)
            visited[neighbor] = true;
            currentPath.push_back(neighbor);
            
            // Recurse
            exhaustiveSearchDFS(graph, neighbor, target, visited, currentPath, 
                                currentDist + edge.weight, bestPath, bestDist, nodesVisited, aborted);
            
            // Step Backward (Backtrack) - This is what makes it Exhaustive!
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
    
    // Initialize Best Distance to Infinity
    double bestDist = std::numeric_limits<double>::infinity();
    long long nodesVisited = 0;
    bool aborted = false;

    // Set up the start node
    visited[start] = true;
    currentPath.push_back(start);

    // Launch the recursion
    exhaustiveSearchDFS(graph, start, target, visited, currentPath, 0.0, bestPath, bestDist, nodesVisited, aborted);

    // Output Handling
    if (aborted) {
        std::cout << "     [WARNING] Exhaustive Search Aborted (Hit 5,000,000 node limit). Too complex!\n";
        appendToCSV(csvFilename, "Exhaustive", graph.V, start, target, nodesVisited, -1.0);
    } 
    else if (bestDist == std::numeric_limits<double>::infinity()) {
        std::cout << "     [RESULT] No path exists.\n";
        appendToCSV(csvFilename, "Exhaustive", graph.V, start, target, nodesVisited, -1.0);
    } 
    else {
        std::cout << "     [SUCCESS] Shortest path perfectly guaranteed!\n";
        std::cout << "     Nodes Visited: " << nodesVisited << "\n";
        std::cout << "     Best Distance: " << bestDist << "\n";
        
        appendToCSV(csvFilename, "Exhaustive", graph.V, start, target, nodesVisited, bestDist);
    }
}

#include <queue>
#include <limits>

struct AStarNode {
    int id;
    double f_cost; // f(n) = g(n) + h(n)

    bool operator>(const AStarNode& other) const {
        return f_cost > other.f_cost; 
    }
};

// THE ENGINE: Pure Math and Pathfinding
void aStarSearchCore(const MapGraph& graph, int start, int target, 
                     std::vector<int>& finalPath, double& totalPathDistance, 
                     long long& nodesVisited) {
    
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> pq;
    
    // A* tracks the absolute best real driving distance to every node
    std::vector<double> gScore(graph.V, std::numeric_limits<double>::infinity());
    std::vector<int> cameFrom(graph.V, -1); 
    
    nodesVisited = 0;
    totalPathDistance = -1.0; 
    finalPath.clear();
    
    // Initialize Start Node
    gScore[start] = 0.0;
    double startH = getDistance(graph.nodes[start], graph.nodes[target]);
    pq.push({start, startH}); // f(start) is just 0 + h(start)

    bool pathFound = false;

    // The Core Loop
    while (!pq.empty()) {
        int current = pq.top().id;
        pq.pop();
        
        nodesVisited++;

        // Target Reached!
        if (current == target) {
            pathFound = true;
            break;
        }

        // Evaluate Neighbors
        for (const Edge& edge : graph.adjList[current]) {
            int neighbor = edge.targetNode;
            
            // tentative_gScore uses edge.weight (which contains our 1.0 - 2.0 Winding Road friction!)
            double tentative_gScore = gScore[current] + edge.weight;
            
            // If we found a physically shorter path to this neighbor, adopt it!
            if (tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                
                // Calculate f(n)
                double h = getDistance(graph.nodes[neighbor], graph.nodes[target]);
                double f = tentative_gScore + h;
                
                pq.push({neighbor, f});
            }
        }
    }

    // Path Reconstruction
    if (pathFound) {
        int curr = target;
        while (curr != -1) {
            finalPath.push_back(curr);
            curr = cameFrom[curr];
        }
        std::reverse(finalPath.begin(), finalPath.end());

        // We don't need a loop to calculate the final distance! 
        // A* already tracked the perfect, friction-adjusted road weight.
        totalPathDistance = gScore[target];
    }
}

// THE WRAPPER: Handles Console Output and CSV Export
void runAStarAndSave(const MapGraph& graph, int start, int target, const std::string& csvFilename) {
    std::cout << "  -> Running A* Search...\n";
    
    std::vector<int> finalPath;
    double totalPathDistance = 0.0;
    long long nodesVisited = 0;

    // 1. Launch the Engine
    aStarSearchCore(graph, start, target, finalPath, totalPathDistance, nodesVisited);

    // 2. Handle the Results
    if (finalPath.empty() || totalPathDistance < 0) {
        std::cout << "     [RESULT] Path Not Found.\n";
        appendToCSV(csvFilename, "A_Star", graph.V, start, target, nodesVisited, -1.0);
    } 
    else {
        std::cout << "     [SUCCESS] A* Path Found!\n";
        std::cout << "     Nodes Visited: " << nodesVisited << "\n";
        std::cout << "     Total Distance: " << totalPathDistance << "\n";
        
        // Save clean data to the CSV
        appendToCSV(csvFilename, "A_Star", graph.V, start, target, nodesVisited, totalPathDistance);
    }
}

int main() {
    // determinate wich file you want to use;
    int sampleRate = 1000;
    int vertex = 1000;

    std::string fileName = "map_"+ std::to_string(vertex) +".txt";
    std::string mapFilename = "data/" + fileName;
    std::string resultFilename = "stress_test_" + std::to_string(vertex) + ".csv";
    
    std::cout << "Loading graph into memory...\n";
    MapGraph myMap = loadGraphFromFile(mapFilename);

    if (myMap.V == 0){
        return 1; // error file empty or doesn't exist
    }; 

    // Open the file in default mode (this INSTANTLY deletes all old data)
    std::ofstream cleaner(resultFilename);
    if (cleaner.is_open()) {
        cleaner << "Algorithm,Vertices,StartNode,TargetNode,NodesVisited,TotalDistance\n";
        cleaner.close();
        std::cout << "Successfully wiped old data and initialized " << resultFilename << "\n";
    }

    // 1. Setup the C++11 Random Engine
    std::random_device rd;  // Obtains a random seed from the hardware
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine
    for(std::size_t i = 0; i < sampleRate; i++){
        // 2. Define the distribution range: [0 to V-1]
        std::uniform_int_distribution<> dist(0, myMap.V - 1);
    
        // 3. Pick the random Start and Target nodes
        int startNode = dist(gen);
        int targetNode = dist(gen);
    
        // 4. Safety Check: Make sure they aren't the exact same node!
        while (startNode == targetNode) {
            targetNode = dist(gen);
        }
    
        std::cout << "\nRandomly selected Start: " << getNodeName(startNode) << " (ID: " << startNode << ")\n";
        std::cout << "Randomly selected Target: " << getNodeName(targetNode) << " (ID: " << targetNode << ")\n";
    
        // // 5. Run the search
        if(vertex < 500){
            runExhaustiveAndSave(myMap, startNode, targetNode, resultFilename);
        }
        runGreedyAndSave(myMap, startNode, targetNode, resultFilename);
        runAStarAndSave(myMap, startNode, targetNode, resultFilename);
    }
    return 0;
}