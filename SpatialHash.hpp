#pragma once
#include <vector>
#include <unordered_map>
#include "Particle.hpp"

class SpatialHash {
public:
    static constexpr float CELL_SIZE = 1.0f;
    
    SpatialHash();
    SpatialHash(size_t size);  // Add new constructor
    
    void update(const std::vector<Particle, AlignedAllocator<Particle>>& particles);
    std::vector<size_t> getNearbyParticles(const Particle& particle, float radius);
    
private:
    std::unordered_map<uint64_t, std::vector<size_t>> grid;
    
    uint64_t hashPosition(const __m256& position);
};
