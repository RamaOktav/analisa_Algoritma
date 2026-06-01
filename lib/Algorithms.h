#pragma once
#include "Graph.h"
#include <string>

void runGreedyAndSave(const MapGraph& graph, int start, int target, const std::string& csvFilename);
void runExhaustiveAndSave(const MapGraph& graph, int start, int target, const std::string& csvFilename);
void runAStarAndSave(const MapGraph& graph, int start, int target, const std::string& csvFilename);