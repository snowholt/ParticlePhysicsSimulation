#include "Simulation.hpp"
#include <thread>
#include <vector>
#include <iostream>
#include <random>
#include <immintrin.h>
#include <algorithm>

Simulation::Simulation(size_t numParticles, float gravityValue, 
                      float initialSpeed, float airFriction) 
    : gravity(gravityValue), 
      initialSpeed(initialSpeed),
      dragCoefficient(airFriction),
      particleHash(numParticles),
      numParticles(numParticles),
      windowWidth(800),    // Add default window width
      windowHeight(600)    // Add default window height
{ 
    std::cout << "Initializing simulation with parameters:" << std::endl
              << "Particles: " << numParticles << std::endl
              << "Gravity: " << gravityValue << std::endl
              << "Speed: " << initialSpeed << std::endl
              << "Friction: " << airFriction << std::endl;
              
    try {
        meshOctree = std::make_unique<Octree>("bunny.obj");
    } catch (const std::exception& e) {
        std::cout << "Warning: Could not load mesh, continuing without it\n";
        // Continue without mesh - it's optional
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posX(-8.0f, 8.0f);
    std::uniform_real_distribution<float> posY(-8.0f, 8.0f);
    std::uniform_real_distribution<float> velDist(-1.0f, 1.0f);
    
    particles.reserve(numParticles);
    for (size_t i = 0; i < numParticles; ++i) {
        Particle p;
        float* position = (float*)&p.position;
        float* velocity = (float*)&p.velocity;
        
        position[0] = posX(gen);
        position[1] = posY(gen);
        position[2] = 0.0f;
        position[3] = 0.0f;
        
        velocity[0] = velDist(gen) * initialSpeed;
        velocity[1] = velDist(gen) * initialSpeed;
        velocity[2] = 0.0f;
        velocity[3] = 0.0f;
        
        p.mass = 1.0f;
        particles.push_back(p);
        
        if (i < 5) {  // Only print first 5 particles
            std::cout << "Particle " << i << " created at (" 
                      << position[0] << ", " << position[1] << ")" << std::endl;
        }
    }
    
    for (auto& particle : particles) {
        float* pos = (float*)&particle.position;
        std::cout << "Particle pos: (" << pos[0] << ", " << pos[1] << ")" << std::endl;
    }
    
    std::cout << "Initialized " << numParticles << " particles\n";
}

void Simulation::update(float deltaTime, float speedMultiplier) {
    try {
        deltaTime *= speedMultiplier;
        
        // Single-threaded update for stability
        updateParticlesBatch(0, particles.size(), deltaTime);
        
        // Update spatial hash after position updates
        particleHash.update(particles);
        
        // Simple collision handling
        for (size_t i = 0; i < particles.size(); ++i) {
            handleParticleCollisions(i, i + 1);
            handleScreenBoundaries(particles[i]);
        }
        
        // Debug output
        if (!particles.empty()) {
            float* pos = (float*)&particles[0].position;
            std::cout << "\rFirst particle at: (" << pos[0] << ", " << pos[1] << ")" << std::flush;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in simulation update: " << e.what() << std::endl;
        throw;
    }
}

void Simulation::updateParticlesBatch(size_t start, size_t end, float deltaTime) {
    try {
        __m256 dt = _mm256_set1_ps(deltaTime);
        __m256 grav = _mm256_set1_ps(gravity);
        
        for (size_t i = start; i < end && i < particles.size(); ++i) {
            Particle& p = particles[i];
            
            // Update velocity with gravity
            p.velocity = _mm256_add_ps(p.velocity, 
                _mm256_mul_ps(_mm256_set_ps(0,0,1,0, 0,0,1,0), grav));
            
            // Update position
            p.position = _mm256_add_ps(p.position, 
                _mm256_mul_ps(p.velocity, dt));
            
            // Apply air resistance
            p.velocity = _mm256_mul_ps(p.velocity, 
                _mm256_set1_ps(1.0f - dragCoefficient * deltaTime));
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in updateParticlesBatch: " << e.what() << std::endl;
        throw;
    }
}

__m256 Simulation::calculateAirResistance(const __m256& velocity, float deltaTime) const {
    // Calculate velocity magnitude squared
    __m256 velSq = _mm256_dp_ps(velocity, velocity, 0x71);
    
    // Calculate drag force magnitude: F = 0.5 * rho * v^2 * Cd * A
    // where rho is air density, v is velocity, Cd is drag coefficient, A is cross-sectional area
    float area = M_PI * PARTICLE_RADIUS * PARTICLE_RADIUS;
    float dragConstant = 0.5f * AIR_DENSITY * dragCoefficient * area;
    __m256 dragForce = _mm256_mul_ps(velSq, _mm256_set1_ps(dragConstant));
    
    // Convert force to velocity change
    __m256 velocityChange = _mm256_mul_ps(
        _mm256_mul_ps(velocity, dragForce),
        _mm256_set1_ps(-deltaTime)
    );
    
    return velocityChange;
}

void Simulation::calculateForcesSIMD() {
    const float coulombConstant = 8.99e9f;
    __m256 k = _mm256_set1_ps(coulombConstant);
    
    for (size_t i = 0; i < particles.size(); ++i) {
        Particle& p1 = particles[i];
        auto nearbyIndices = particleHash.getNearbyParticles(p1, 5.0f);
        
        __m256 force_sum = _mm256_setzero_ps();
        __m256 p1_pos = p1.position;
        __m256 p1_charge = _mm256_set1_ps(p1.charge);
        
        for (size_t j : nearbyIndices) {
            if (i == j) continue;
            
            Particle& p2 = particles[j];
            __m256 p2_pos = p2.position;
            __m256 p2_charge = _mm256_set1_ps(p2.charge);
            
            // Calculate distance vector
            __m256 dist_vec = _mm256_sub_ps(p2_pos, p1_pos);
            
            // Calculate distance squared
            __m256 dist_sq = _mm256_dp_ps(dist_vec, dist_vec, 0x71);
            
            // Prevent division by zero
            dist_sq = _mm256_add_ps(dist_sq, _mm256_set1_ps(1e-6f));
            
            // Calculate Coulomb force
            __m256 force_mag = _mm256_div_ps(
                _mm256_mul_ps(_mm256_mul_ps(k, p1_charge), p2_charge),
                _mm256_mul_ps(dist_sq, _mm256_sqrt_ps(dist_sq))
            );
            
            // Add to force sum
            force_sum = _mm256_add_ps(force_sum,
                _mm256_mul_ps(dist_vec, _mm256_broadcast_ss((float*)&force_mag))
            );
        }
        
        // Update velocity based on force
        p1.velocity = _mm256_add_ps(p1.velocity, force_sum);
    }
}

void Simulation::handleCollisions() {
    const float restitution = 0.8f;
    __m256 rest = _mm256_set1_ps(restitution);
    
    for (auto& particle : particles) {
        if (meshOctree->checkCollision(particle.position, 0.1f)) {
            // Simple bounce - invert velocity with restitution
            particle.velocity = _mm256_mul_ps(
                _mm256_mul_ps(particle.velocity, _mm256_set1_ps(-1.0f)),
                rest
            );
        }
    }
}

void Simulation::handleScreenBoundaries(Particle& p) {
    float* pos = (float*)&p.position;
    float* vel = (float*)&p.velocity;
    
    // X boundaries (left and right)
    if (pos[0] < SCREEN_LEFT) {
        pos[0] = SCREEN_LEFT;
        vel[0] = -vel[0] * BOUNCE_FACTOR;
    } else if (pos[0] > SCREEN_RIGHT) {
        pos[0] = SCREEN_RIGHT;
        vel[0] = -vel[0] * BOUNCE_FACTOR;
    }
    
    // Y boundaries (top and bottom)
    if (pos[1] < SCREEN_BOTTOM) {
        pos[1] = SCREEN_BOTTOM;
        vel[1] = -vel[1] * BOUNCE_FACTOR;
    } else if (pos[1] > SCREEN_TOP) {
        pos[1] = SCREEN_TOP;
        vel[1] = -vel[1] * BOUNCE_FACTOR;
    }
    
    // Apply additional drag when hitting boundaries to simulate friction
    if (pos[1] == SCREEN_BOTTOM || pos[1] == SCREEN_TOP || 
        pos[0] == SCREEN_LEFT || pos[0] == SCREEN_RIGHT) {
        vel[0] *= 0.98f; // Horizontal friction
        vel[2] *= 0.98f; // Z-axis friction
    }
}

void Simulation::handleParticleCollisions(size_t startIdx, size_t endIdx) {
    for (size_t i = startIdx; i < endIdx; ++i) {
        Particle& p1 = particles[i];
        auto nearbyIndices = particleHash.getNearbyParticles(p1, PARTICLE_RADIUS * 2.0f);
        
        for (size_t j : nearbyIndices) {
            if (i >= j) continue; // Avoid double-checking pairs
            
            Particle& p2 = particles[j];
            if (checkParticleCollision(p1, p2)) {
                resolveParticleCollision(p1, p2);
            }
        }
    }
}

bool Simulation::checkParticleCollision(const Particle& p1, const Particle& p2) {
    __m256 diff = _mm256_sub_ps(p1.position, p2.position);
    __m256 distSq = _mm256_dp_ps(diff, diff, 0x71);
    float dist;
    _mm256_store_ps(&dist, distSq);
    return dist < (PARTICLE_RADIUS * 2.0f) * (PARTICLE_RADIUS * 2.0f);
}

void Simulation::resolveParticleCollision(Particle& p1, Particle& p2) {
    // Calculate collision normal
    __m256 diff = _mm256_sub_ps(p2.position, p1.position);
    __m256 distSq = _mm256_dp_ps(diff, diff, 0x71);
    float dist;
    _mm256_store_ps(&dist, distSq);
    dist = std::sqrt(dist);
    
    // Normalize the difference to get collision normal
    __m256 normal = _mm256_div_ps(diff, _mm256_set1_ps(dist));
    
    // Calculate relative velocity
    __m256 relativeVel = _mm256_sub_ps(p2.velocity, p1.velocity);
    
    // Calculate relative velocity along normal
    __m256 velAlongNormal = _mm256_dp_ps(relativeVel, normal, 0x71);
    float relativeSpeed;
    _mm256_store_ps(&relativeSpeed, velAlongNormal);
    
    // Only resolve collision if particles are moving toward each other
    if (relativeSpeed < 0) {
        // Calculate impulse scalar
        __m256 impulse = _mm256_mul_ps(normal, 
            _mm256_set1_ps(-relativeSpeed * (1.0f + BOUNCE_FACTOR) * 0.5f));
        
        // Apply impulse
        p1.velocity = _mm256_sub_ps(p1.velocity, impulse);
        p2.velocity = _mm256_add_ps(p2.velocity, impulse);
    }
}