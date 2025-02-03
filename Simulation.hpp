#pragma once
#include <vector>
#include <memory>
#include "Particle.hpp"
#include "Octree.hpp"
#include "SpatialHash.hpp"

class Simulation {
public:
    Simulation(size_t numParticles, float gravityValue = -9.81f, 
              float initialSpeed = 1.0f, float airFriction = 0.47f);
    void update(float deltaTime, float speedMultiplier = 1.0f);
    const std::vector<Particle, AlignedAllocator<Particle>>& getParticles() const {
        return particles;
    }

private:
    static constexpr size_t THREAD_COUNT = 8;
    static constexpr float BASE_AIR_RESISTANCE = 0.01f;  // Base air resistance coefficient
    static constexpr float AIR_DENSITY = 1.225f;         // kg/m^3 at sea level
    static constexpr float BOUNCE_FACTOR = 0.8f;  // Increased bounce factor
    static constexpr float FLOOR_Y = -10.0f;      // Floor position
    
    // Update screen boundaries to match viewport exactly
    static constexpr float SCREEN_LEFT = -13.333f;
    static constexpr float SCREEN_RIGHT = 13.333f;
    static constexpr float SCREEN_TOP = 10.0f;
    static constexpr float SCREEN_BOTTOM = -10.0f;
    static constexpr float SCREEN_NEAR = -1.0f;
    static constexpr float SCREEN_FAR = 1.0f;
    static constexpr float PARTICLE_RADIUS = 0.3f;  // Increased particle size
    
    float gravity;
    float initialSpeed;
    float dragCoefficient;  // Now a member variable instead of constant
    std::vector<Particle, AlignedAllocator<Particle>> particles;
    std::unique_ptr<Octree> meshOctree;
    SpatialHash particleHash;
    
    int numParticles;
    int windowWidth;
    int windowHeight;

    void updateParticlesBatch(size_t start, size_t end, float deltaTime);
    void calculateForcesSIMD();
    void handleCollisions();
    
    void handleScreenBoundaries(Particle& p);
    void handleParticleCollisions(size_t startIdx, size_t endIdx);
    bool checkParticleCollision(const Particle& p1, const Particle& p2);
    void resolveParticleCollision(Particle& p1, Particle& p2);
    
    // SIMD helper methods
    static inline float getY(__m256 v) {
        float tmp[8];
        _mm256_store_ps(tmp, v);
        return tmp[1];
    }
    
    static inline void setY(__m256& v, float y) {
        float tmp[8];
        _mm256_store_ps(tmp, v);
        tmp[1] = y;
        v = _mm256_load_ps(tmp);
    }
    
    static inline __m256 scaleVector(__m256 v, float scale) {
        return _mm256_mul_ps(v, _mm256_set1_ps(scale));
    }

    // Helper for calculating air resistance
    __m256 calculateAirResistance(const __m256& velocity, float deltaTime) const;
};
