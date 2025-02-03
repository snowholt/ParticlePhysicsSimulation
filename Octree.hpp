#pragma once
#include <immintrin.h>
#include <vector>
#include <array>

class Octree {
public:
    explicit Octree(const char* name);
    bool checkCollision(const __m256& position, float radius) const;
    
private:
    struct Node {
        std::array<Node*, 8> children{nullptr};
        std::vector<unsigned int> triangles;
        bool isLeaf{true};
    };
    
    Node* root;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};
