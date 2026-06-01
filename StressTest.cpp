// change for creating log for the start and target
#define LOG_STRESS 0
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "lib/Graph.h"
#include "lib/FileIO.h"
#include "lib/Algorithms.h"


std::vector<int> generateVertexSizes() {
    std::vector<int> vertices;
    int v = 16;
    while(v <= 50000){
        vertices.push_back(v);
        if(v < 500) v = static_cast<int>(v * 1.2);
        else v *= 2;
    }
    return vertices;
}

int getSampleRate(int v) {
    if (v < 100) return 500;   // High resolution for small maps
    if (v < 1000) return 100;  // Medium resolution
    if (v < 50000) return 100;  // Low resolution
    return 5;                  // Minimal samples for massive maps
}


int main() {
    // 0 = Debug (Fast, small samples)
    // 1 = Full Stress Test (Aggressive, multi-stage)
    const int BENCHMARK_MODE = 0; 

    std::vector<int> vertices;

    if (BENCHMARK_MODE == 0) {
        vertices = {15, 100, 500, 1000};
    } else {
        vertices = generateVertexSizes();
    }

    std::random_device rd;  // Obtains a random seed from the hardware
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine

    
    for(int v_count : vertices){
        int currentSampleRate = getSampleRate(v_count);
        std::cout << "\n>>> BENCHMARK: V=" << v_count << " | Samples: " << currentSampleRate << "\n";

        std::string fileName = "map_"+ std::to_string(v_count) +".txt";
        std::string mapFilename = "data/" + fileName;
        std::cout << "Loading graph into memory...\n";
        MapGraph myMap = loadGraphFromFile(mapFilename);
        std::string resultFilename = "result/stress_test_" + std::to_string(v_count) + ".csv";

        if (myMap.V == 0){
            std::cerr << "[WARNING] Missing file " << mapFilename << ". Skipping to next map.\n";
            continue; 
        };

        // Open the file in default mode (this INSTANTLY deletes all old data)
        initializeCSV(resultFilename);
        std::cout << "Successfully wiped old data and initialized " << resultFilename << "\n";


        for(std::size_t i = 0; i < currentSampleRate; i++){
            std::uniform_int_distribution<> dist(0, myMap.V - 1);
            int startNode = dist(gen);
            int targetNode = dist(gen);
        
            while (startNode == targetNode) {
                targetNode = dist(gen);
            }

            if(LOG_STRESS){
                std::cout << "\n[Test " << i+1 << "/" << currentSampleRate << "] Start: " << startNode << " -> Target: " << targetNode << "\n";
            }
        
            if(v_count < 500){
                runExhaustiveAndSave(myMap, startNode, targetNode, resultFilename);
            }
            runGreedyAndSave(myMap, startNode, targetNode, resultFilename);
            runAStarAndSave(myMap, startNode, targetNode, resultFilename);
        }
    }

    std::cout << "\nALL BENCHMARKS COMPLETED SUCCESSFULLY.\n";

    return 0;
}