#include "Renderer.hpp"
#include <stdexcept>
#include <iostream>
#include <cmath>

// GLFW error callback
static void errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

Renderer::Renderer(int w, int h) : width(w), height(h) {
    try {
        // Initialize GLFW with error callback
        glfwSetErrorCallback(errorCallback);
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        
        // Configure GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        // Create window
        window = glfwCreateWindow(width, height, "Particle Simulation", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }
        
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync
        
        // Initialize OpenGL
        initializeGL();
        
        // Set up keyboard callback
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallback);
        
    } catch (const std::exception& e) {
        std::cerr << "Error in Renderer constructor: " << e.what() << std::endl;
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
        throw;
    }
}

Renderer::~Renderer() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void Renderer::initializeGL() {
    try {
        // Basic OpenGL setup
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        
        // Get actual framebuffer size
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        
        // Set up projection matrix
        float aspectRatio = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
        float viewHeight = 20.0f;
        float viewWidth = viewHeight * aspectRatio;
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-viewWidth/2, viewWidth/2, -viewHeight/2, viewHeight/2, -1.0, 1.0);
        
        // Reset to modelview matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Check for errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL initialization error: " << err << std::endl;
        }
        
        std::cout << "OpenGL initialized with viewport: " 
                  << -viewWidth/2 << " to " << viewWidth/2 << " (X), "
                  << -viewHeight/2 << " to " << viewHeight/2 << " (Y)" << std::endl;
                  
    } catch (const std::exception& e) {
        std::cerr << "Error in initializeGL: " << e.what() << std::endl;
        throw;
    }
}

void Renderer::render(const std::vector<Particle, AlignedAllocator<Particle>>& particles) {
    try {
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        
        glPointSize(10.0f);  // Make particles larger
        glBegin(GL_POINTS);
        
        for (const auto& particle : particles) {
            const float* pos = (float*)&particle.position;
            
            // Simple white color for visibility
            glColor3f(1.0f, 1.0f, 1.0f);
            
            // Only draw if position is valid
            if (std::isfinite(pos[0]) && std::isfinite(pos[1])) {
                glVertex2f(pos[0], pos[1]);
            }
        }
        glEnd();
        
        glFlush();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    catch (const std::exception& e) {
        std::cerr << "Error in render: " << e.what() << std::endl;
    }
}

bool Renderer::shouldClose() const {
    return glfwWindowShouldClose(window);
}

bool Renderer::isKeyPressed(char key) const {
    return glfwGetKey(window, static_cast<int>(std::toupper(key))) == GLFW_PRESS;
}

void Renderer::keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}