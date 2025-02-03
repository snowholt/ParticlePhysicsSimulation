# 🎨 Particle Physics Simulation

A high-performance particle physics simulation using SIMD instructions and OpenGL.

![Particle Simulation Demo](demo.gif) <!-- You can add a demo gif later -->

## ✨ Features

- 🚀 SIMD-accelerated physics calculations
- 💫 Real-time particle interactions
- 🎯 Elastic collisions with boundaries
- 🌈 Dynamic particle visualization
- 🔄 Spatial hashing for efficient collision detection
- 🎮 Interactive controls

## 🛠️ Technical Details

- **CPU Optimization**: Uses AVX2 SIMD instructions for parallel processing
- **Memory Management**: Custom aligned allocator for SIMD operations
- **Physics**: 
  - Gravitational forces
  - Air resistance
  - Elastic collisions
  - Boundary interactions
- **Graphics**: OpenGL with GLFW for rendering

## 🚀 Building and Running

```bash
# Clone the repository
git clone https://github.com/yourusername/particle-sim.git

# Build the project
./build.sh

# Run the simulation
./particle_sim
```

## 🎮 Controls

- **ESC**: Exit simulation
- **Number Input**: Set particle count at startup
- **Parameters**:
  - Gravity strength
  - Initial particle speed
  - Air friction coefficient

## 🖥️ System Requirements

- C++17 compatible compiler
- OpenGL 2.1+
- GLFW3
- CPU with AVX2 support

## 📊 Performance

The simulation efficiently handles thousands of particles with:
- SIMD-optimized physics calculations
- Spatial partitioning for collision detection
- Multi-threaded updates

## 🎨 Visualization

- Particles rendered as smooth points
- Color variation based on velocity
- Real-time position updates
- Smooth boundary interactions

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Feel free to:
1. Fork the repository
2. Create a feature branch
3. Submit a pull request

## 🌟 Acknowledgments

- SIMD implementation inspired by modern CPU architecture
- Particle physics based on classical mechanics
- Rendering optimized for real-time visualization

---
Made with ❤️ and lots of ⚛️ particles
