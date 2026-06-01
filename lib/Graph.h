#pragma once
#include <vector>
#include <string>

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

std::string getNodeName(int id);
double getDistance(const Coordinate& a, const Coordinate& b);
double getDistance(const Coordinate& a, const Coordinate& b);