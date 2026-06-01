#include "Graph.h"
#include <cmath>

std::string getNodeName(int id) {
    char letter = 'A' + (id % 26);
    int number = (id / 26) + 1;
    return std::string(1, letter) + std::to_string(number);
}

double getDistance(const Coordinate& a, const Coordinate& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}