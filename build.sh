
#!/bin/bash

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make -j$(nproc)

# Copy executable to parent directory
cp particle_sim ..