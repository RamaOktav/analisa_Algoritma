#pragma once
#include "Graph.h"
#include <string>
#include <filesystem>

MapGraph loadGraphFromFile(const std::string& filename);
void initializeCSV(const std::string& filename);
void appendToCSV(const std::string& filename, const std::string& algo, int V, int start, int target, int nodesVisited, double distance);