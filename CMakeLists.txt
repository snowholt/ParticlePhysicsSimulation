cmake_minimum_required(VERSION 3.10)
project(ParticleSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable optimizations and AVX instructions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mavx -mavx2 -march=native -pthread")

# Add warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Add pkg-config support
find_package(PkgConfig REQUIRED)

# Find required packages
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
pkg_check_modules(GLFW REQUIRED glfw3)

# Add source files
add_executable(particle_sim
    main.cpp
    Simulation.cpp
    PerformanceMonitor.cpp
    SpatialHash.cpp
    Octree.cpp
    Renderer.cpp
)

# Add include directories
target_include_directories(particle_sim PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
)

# Find and link necessary libraries
find_package(Threads REQUIRED)
target_link_libraries(particle_sim PRIVATE 
    Threads::Threads
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
)

# Add link directories
target_link_directories(particle_sim PRIVATE
    ${GLFW_LIBRARY_DIRS}
)