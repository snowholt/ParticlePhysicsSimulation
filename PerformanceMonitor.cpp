#include "PerformanceMonitor.hpp"
#include <iostream>
#include <numeric>
#include <algorithm>  // Add this header
#include <sys/resource.h>

void PerformanceMonitor::beginFrame() {
    startTime = std::chrono::high_resolution_clock::now();
    
    // Update peak memory usage
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    float currentMemory = static_cast<float>(usage.ru_maxrss) / 1024.0f; // Convert to MB
    peakMemoryUsage = std::max(peakMemoryUsage, currentMemory);
}

void PerformanceMonitor::endFrame() {
    auto endTime = std::chrono::high_resolution_clock::now();
    float frameTime = std::chrono::duration<float>(endTime - startTime).count() * 1000.0f; // Convert to ms
    frameTimes.push_back(frameTime);
}

float PerformanceMonitor::getRuntime() const {
    if (frameTimes.empty()) return 0.0f;
    return std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f) / 1000.0f; // Convert to seconds
}

void PerformanceMonitor::printMetrics() const {
    if (frameTimes.empty()) {
        std::cout << "No performance data available.\n";
        return;
    }

    // Calculate average FPS
    float totalTime = getRuntime();
    float avgFPS = static_cast<float>(frameTimes.size()) / totalTime;

    // Calculate frame time statistics
    float avgFrameTime = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f) / frameTimes.size();
    float minFrameTime = *std::min_element(frameTimes.begin(), frameTimes.end());
    float maxFrameTime = *std::max_element(frameTimes.begin(), frameTimes.end());

    // Print metrics
    std::cout << "\nPerformance Metrics:\n";
    std::cout << "==================\n";
    std::cout << "Runtime: " << totalTime << " seconds\n";
    std::cout << "Average FPS: " << avgFPS << "\n";
    std::cout << "Frame Times (ms):\n";
    std::cout << "  Average: " << avgFrameTime << "\n";
    std::cout << "  Min: " << minFrameTime << "\n";
    std::cout << "  Max: " << maxFrameTime << "\n";
    std::cout << "Peak Memory Usage: " << peakMemoryUsage << " MB\n";
    std::cout << "Total Frames: " << frameTimes.size() << "\n";
}