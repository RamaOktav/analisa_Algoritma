#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "lib/Graph.h"
#include "lib/FileIO.h"
#include "lib/Algorithms.h"

int main() {
    // determinate wich file you want to use;
    std::vector<int> sampleRate = {1000, 1000, 1000, 1000};
    std::vector<int> vertex = {15, 100, 500, 1000};

    std::random_device rd;  // Obtains a random seed from the hardware
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine

    
    for(std::size_t v = 0; v < sampleRate.size(); ++v){
        std::cout << "STARTING BENCHMARK FOR V = " << vertex[v] << "\n";

        std::string fileName = "map_"+ std::to_string(vertex[v]) +".txt";
        std::string mapFilename = "data/" + fileName;
        std::cout << "Loading graph into memory...\n";
        MapGraph myMap = loadGraphFromFile(mapFilename);
        std::string resultFilename = "result/stress_test_" + std::to_string(vertex[v]) + ".csv";

        if (myMap.V == 0){
            std::cerr << "[WARNING] Missing file " << mapFilename << ". Skipping to next map.\n";
            continue; 
        };

        // Open the file in default mode (this INSTANTLY deletes all old data)
        initializeCSV(resultFilename);
        std::cout << "Successfully wiped old data and initialized " << resultFilename << "\n";


        for(std::size_t i = 0; i < sampleRate[v]; i++){
            std::uniform_int_distribution<> dist(0, myMap.V - 1);
            int startNode = dist(gen);
            int targetNode = dist(gen);
        
            while (startNode == targetNode) {
                targetNode = dist(gen);
            }

            std::cout << "\n[Test " << i+1 << "/" << sampleRate[v] << "] Start: " << startNode << " -> Target: " << targetNode << "\n";
        
            if(vertex[v] < 500){
                runExhaustiveAndSave(myMap, startNode, targetNode, resultFilename);
            }
            runGreedyAndSave(myMap, startNode, targetNode, resultFilename);
            runAStarAndSave(myMap, startNode, targetNode, resultFilename);
        }
    }

    std::cout << "\nALL BENCHMARKS COMPLETED SUCCESSFULLY.\n";

    return 0;
}