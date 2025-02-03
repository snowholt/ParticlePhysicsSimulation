#include <chrono>
#include <iostream>
#include "Simulation.hpp"
#include "PerformanceMonitor.hpp"
#include "Renderer.hpp"

int main() {
    // Get gravity input
    float gravity;
    std::cout << "Enter gravity value (default -9.81): ";
    std::string input;
    std::getline(std::cin, input);
    gravity = input.empty() ? -9.81f : std::stof(input);

    // Get initial particle speed
    float initialSpeed;
    std::cout << "Enter initial particle speed (default 1.0): ";
    std::getline(std::cin, input);
    initialSpeed = input.empty() ? 1.0f : std::stof(input);

    // Get air friction coefficient
    float airFriction;
    std::cout << "Enter air friction coefficient (0.0-1.0, default 0.47): ";
    std::getline(std::cin, input);
    airFriction = input.empty() ? 0.47f : std::stof(input);

    // Get particle count without arbitrary limits
    size_t numParticles;
    do {
        std::cout << "Enter the number of particles: ";
        std::cin >> numParticles;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        break;
    } while (true);

    try {
        Simulation sim(numParticles, gravity, initialSpeed, airFriction);
        PerformanceMonitor perfMon;
        Renderer renderer;
        
        std::cout << "Initializing Particle Simulation...\n";
        
        float speedMultiplier = 1.0f;
        while (!renderer.shouldClose()) {
            perfMon.beginFrame();
            
            // Speed control with keyboard
            if (renderer.isKeyPressed('Q')) speedMultiplier *= 1.1f;
            if (renderer.isKeyPressed('E')) speedMultiplier *= 0.9f;
            
            sim.update(1.0f / 60.0f, speedMultiplier);
            renderer.render(sim.getParticles());
            
            perfMon.endFrame();
        }
        
        perfMon.printMetrics();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
