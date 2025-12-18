# NitroClimb 🏎️

A 3D hill climb racing game built with OpenGL and Bullet Physics, featuring realistic physics simulation, collectible items, and progressive difficulty.

## 🎮 Game Overview

**NitroClimb** is an exciting hill climb racing game where you drive various vehicles across procedurally generated terrain, collecting coins, fuel, and nitro boosts while managing your resources to travel as far as possible.

### Game Features

- **Three Interactive Scenes:**

  - **Car Selection Scene**: Choose from multiple vehicles with an interactive circular platform display
  - **Game Scene**: Race across infinite procedurally generated terrain
  - **Game Over Scene**: View your final score and restart or exit

- **Realistic Physics Simulation**: Powered by Bullet Physics engine for authentic vehicle dynamics

- **Collectible Items:**

  - 💰 **Coins**: Collect to increase your score
  - ⛽ **Fuel**: Keep your vehicle running
  - 🚀 **Nitro Boost**: Activate for temporary speed bursts

- **Progressive Difficulty**: Terrain becomes more challenging as you progress

- **Dynamic Camera System**: Smooth third-person camera that follows your vehicle

- **Procedural Terrain Generation**: Infinite, randomly generated hills for endless gameplay

## 📹 Demo Video

Watch the gameplay in action! (30-second screen recording)

## 📸 Screenshots

### Car Selection Scene

_Choose from multiple vehicles on an interactive rotating platform_

### Gameplay

_Navigate procedurally generated terrain while collecting items_

### Game Over Screen

_View your final score and choose to continue or exit_

## 🎯 Controls

- **WASD**: Accelerate, brake, and balance your vehicle
- **Shift**: Activate nitro boost (when available)
- **ESC**: Exit
- **Mouse**: Navigate menus (car selection and game over screens)

## 🛠️ Prerequisites

- CMake (version 3.0 or higher)
- C++17 compatible compiler
- Homebrew (for macOS)

## 📦 Dependencies

This project requires the following libraries:

- **OpenGL** - Graphics rendering
- **GLFW3** - Window and input management
- **GLM** - Mathematics library for 3D transformations
- **Assimp** - 3D model loading
- **Freetype** - Font rendering for UI text
- **Bullet Physics** - Physics simulation engine

## 🚀 Installation

### macOS

1. **Install dependencies:**

   ```bash
   brew install bullet glfw glm assimp freetype
   ```

2. **Clone the repository:**

   ```bash
   git clone <repository-url>
   cd OpenGLProject
   ```

3. **Build the project:**

   ```bash
   cd build
   cmake ..
   make
   ```

4. **Run the game:**

   ```bash
   cd ../bin/game_project
   ./game_project
   ```

## 🎨 Project Structure

```
OpenGLProject/
├── src/game_project/
│   ├── core/           # Core game logic (car, collectibles, controls)
│   ├── physics/        # Physics simulation (Bullet integration)
│   ├── scene/          # Scene management and terrain generation
│   ├── ui/             # User interface rendering
│   ├── input/          # Input handling
│   └── main.cpp        # Main game loop
├── resources/
│   ├── objects/        # 3D models
│   ├── images/         # Textures and UI images
│   └── fonts/          # Font files for text rendering
└── bin/game_project/   # Compiled executable and shaders
```

## 🎮 Gameplay

1. **Car Selection**: Use left/right arrows or mouse to browse available vehicles, then press ENTER or click to start
2. **Drive**: Navigate the procedurally generated terrain while managing fuel
3. **Collect Items**:
   - Coins increase your score
   - Fuel keeps you going
   - Nitro provides temporary speed boosts
4. **Survive**: The game ends when you run out of fuel
5. **Retry**: View your score and choose to continue or exit

## 🏆 Credits

### Development

- Built with OpenGL, GLFW, GLM, Assimp, Freetype, and Bullet Physics

### 3D Models & Assets

- **Sky Background**: [Blue Tropical Watercolor Background](https://www.freepik.com/free-photo/abstract-refreshing-blue-tropical-watercolor-background-illustration-high-resolution-free-image_26538625.htm) by Freepik
- **Speedometer Icon**: [PNG Images](https://pngimg.com/image/28800)
- **Nitro Object**: [NOS Bottle 3D Model](https://skfb.ly/oRWzz) by Sketchfab
- **Nitro Icon**: [NOS Icon](https://www.freepik.com/icon/nos_2041104) by Freepik
- **Coin Icon**: [Coin Icon](https://www.flaticon.com/free-icon/coin_217802) by Flaticon
- **Fuel Icon**: [Fuel Icon](https://www.flaticon.com/free-icon/fuel_8664993) by Flaticon
- **Coin Object**: [Gold Coin Hand Painted](https://www.cgtrader.com/free-3d-models/various/various-models/gold-coin-hand-painted) by CGTrader
- **BMW E30 Model**: [BMW E30 3D Model](https://skfb.ly/o8EBR) by Sketchfab

## 📄 License

This project is for educational purposes. All third-party assets are credited to their respective owners.

## 🤝 Contributing

This is an educational project. Feel free to fork and experiment!

---

**Enjoy the climb! 🏔️🚗**
