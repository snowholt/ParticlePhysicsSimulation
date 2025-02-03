#include "SpatialHash.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

SpatialHash::SpatialHash() {
    std::cout << "Creating default SpatialHash" << std::endl;
    grid.reserve(1000);
}

SpatialHash::SpatialHash(size_t size) {
    std::cout << "Creating SpatialHash with size " << size << std::endl;
    grid.reserve(size);
}

void SpatialHash::update(const std::vector<Particle, AlignedAllocator<Particle>>& particles) {
    try {
        grid.clear();
        for (size_t i = 0; i < particles.size(); ++i) {
            const float* pos = reinterpret_cast<const float*>(&particles[i].position);
            if (!pos) {
                throw std::runtime_error("Invalid position pointer");
            }
            
            // Validate position values
            if (std::isnan(pos[0]) || std::isnan(pos[1]) || std::isnan(pos[2])) {
                std::cerr << "Warning: NaN position detected for particle " << i << std::endl;
                continue;
            }
            
            uint64_t hash = hashPosition(particles[i].position);
            grid[hash].push_back(i);
        }
        
        // Debug output
        if (!particles.empty()) {
            const float* firstPos = reinterpret_cast<const float*>(&particles[0].position);
            std::cout << "\rFirst particle at: (" << firstPos[0] << ", " << firstPos[1] 
                      << ", " << firstPos[2] << ")" << std::flush;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in SpatialHash::update: " << e.what() << std::endl;
        throw;
    }
}

std::vector<size_t> SpatialHash::getNearbyParticles(const Particle& particle, float radius) {
    std::vector<size_t> nearby;
    nearby.reserve(27); // Reserve space for 3x3x3 neighborhood
    
    try {
        const float* pos = reinterpret_cast<const float*>(&particle.position);
        if (!pos) {
            throw std::runtime_error("Invalid position pointer in getNearbyParticles");
        }
        
        // Calculate cell range based on radius
        int cellRadius = static_cast<int>(std::ceil(radius / CELL_SIZE));
        
        // Get base cell coordinates
        int baseX = static_cast<int>(std::floor(pos[0] / CELL_SIZE));
        int baseY = static_cast<int>(std::floor(pos[1] / CELL_SIZE));
        
        // Check neighboring cells
        for (int x = -cellRadius; x <= cellRadius; ++x) {
            for (int y = -cellRadius; y <= cellRadius; ++y) {
                int cellX = baseX + x;
                int cellY = baseY + y;
                
                uint64_t hash = (static_cast<uint64_t>(cellX) << 32) | 
                               static_cast<uint64_t>(cellY);
                
                auto it = grid.find(hash);
                if (it != grid.end()) {
                    nearby.insert(nearby.end(), it->second.begin(), it->second.end());
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in getNearbyParticles: " << e.what() << std::endl;
        throw;
    }
    
    return nearby;
}

uint64_t SpatialHash::hashPosition(const __m256& position) {
    const float* pos = reinterpret_cast<const float*>(&position);
    if (!pos) {
        throw std::runtime_error("Invalid position pointer in hashPosition");
    }
    
    // Add bounds checking
    if (std::isnan(pos[0]) || std::isnan(pos[1])) {
        throw std::runtime_error("NaN position detected in hashPosition");
    }
    
    // Convert position to cell coordinates
    int x = static_cast<int>(std::floor(pos[0] / CELL_SIZE));
    int y = static_cast<int>(std::floor(pos[1] / CELL_SIZE));
    
    // Combine x and y into a single 64-bit hash
    return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(y);
}