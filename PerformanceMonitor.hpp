#pragma once
#include <chrono>
#include <vector>

class PerformanceMonitor {
public:
    void beginFrame();
    void endFrame();
    float getRuntime() const;
    void printMetrics() const;
    
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::vector<float> frameTimes;
    float peakMemoryUsage = 0.0f;
    size_t collisionCount = 0;
    float initialEnergy = 0.0f;
};
