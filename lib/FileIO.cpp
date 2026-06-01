#include "FileIO.h"
#include <fstream>
#include <iostream>
#include <filesystem>

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

    for (int i = 0; i < totalEdges; i++) {
        int u, v; 
        double weight;
        inFile >> u >> v >> weight;
        graph.adjList[u].push_back({v, weight});
        graph.adjList[v].push_back({u, weight});
    }

    inFile.close();
    return graph;
}

void initializeCSV(const std::string& filename) {
    std::filesystem::path filePath(filename);
    std::filesystem::path dir = filePath.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
        std::cout << "Created missing directory: " << dir << "\n";
    }
    
    std::ofstream outFile(filename); 
    if (outFile.is_open()) {
        outFile << "Algorithm,Vertices,StartNode,TargetNode,NodesVisited,TotalDistance\n";
        outFile.close();
    }
}

void appendToCSV(const std::string& filename, const std::string& algo, int V, int start, int target, int nodesVisited, double distance) {
    std::ofstream outFile;
    outFile.open(filename, std::ios::app); 
    
    if (outFile.is_open()) {
        outFile << algo << "," << V << "," << start << "," << target << "," << nodesVisited << "," << distance << "\n";
        outFile.close();
    } else {
        std::cerr << "CRITICAL ERROR: Could not open " << filename << " to append data.\n";
    }
}