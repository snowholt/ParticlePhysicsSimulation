#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include "Particle.hpp"

class Renderer {
public:
    Renderer(int width = 1024, int height = 768);
    ~Renderer();
    
    void render(const std::vector<Particle, AlignedAllocator<Particle>>& particles);
    bool shouldClose() const;
    bool isKeyPressed(char key) const;
    
private:
    GLFWwindow* window;
    int width, height;
    void initializeGL();
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
