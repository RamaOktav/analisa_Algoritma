#include <vector>
#include <cmath>
#include <iostream>

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
    double baseSize = 500.0;
    double baseVertices = 100.0;
    
    return baseSize * std::sqrt((double)vertices / baseVertices);
}

int main(void){
    std::vector<int> size = generateVertexSizes();
    for(int num : size){
        std::cout << num << " : " << XYsize(num) << std::endl;
    } 
}