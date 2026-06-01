#include <iostream>
#include <random>
#include <string>
#include "lib/Graph.h"
#include "lib/FileIO.h"
#include "lib/Algorithms.h"

int main() {
    // determinate wich file you want to use;
    int sampleRate[] = {1000, 1000, 1000, 1000};
    int vertex[] = {15, 100, 500, 1000};

    std::string fileName = "map_"+ std::to_string(vertex) +".txt";
    std::string mapFilename = "data/" + fileName;
    std::string resultFilename = "result/stress_test_" + std::to_string(vertex) + ".csv";
    
    std::cout << "Loading graph into memory...\n";
    MapGraph myMap = loadGraphFromFile(mapFilename);

    if (myMap.V == 0){
        return 1; 
    }; 

    // Open the file in default mode (this INSTANTLY deletes all old data)
    initializeCSV(resultFilename);
    std::cout << "Successfully wiped old data and initialized " << resultFilename << "\n";

    std::random_device rd;  // Obtains a random seed from the hardware
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine

    for(std::size_t i = 0; i < sampleRate; i++){
        std::uniform_int_distribution<> dist(0, myMap.V - 1);
        int startNode = dist(gen);
        int targetNode = dist(gen);
    
        while (startNode == targetNode) {
            targetNode = dist(gen);
        }
    
        std::cout << "\nRandomly selected Start: " << getNodeName(startNode) << " (ID: " << startNode << ")\n";
        std::cout << "Randomly selected Target: " << getNodeName(targetNode) << " (ID: " << targetNode << ")\n";
    
        if(vertex < 500){
            runExhaustiveAndSave(myMap, startNode, targetNode, resultFilename);
        }
        runGreedyAndSave(myMap, startNode, targetNode, resultFilename);
        runAStarAndSave(myMap, startNode, targetNode, resultFilename);
    }
    return 0;
}