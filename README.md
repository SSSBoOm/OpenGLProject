# OpenGL Car Physics Project

A 3D car simulation project using OpenGL for rendering and Bullet Physics for realistic physics simulation.

## Prerequisites

- CMake (version 3.0 or higher)
- C++17 compatible compiler
- Homebrew (for macOS)

## Dependencies

This project requires the following libraries:

- **OpenGL** - Graphics rendering
- **GLFW3** - Window and input management
- **GLM** - Mathematics library
- **Assimp** - 3D model loading
- **Freetype** - Font rendering
- **Bullet Physics** - Physics simulation

## Installation

### macOS

1. **Install Bullet Physics:**

   ```bash
   brew install bullet
   ```

2. **Install other dependencies (if not already installed):**

   ```bash
   brew install glfw glm assimp freetype
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

### Linux

1. **Install Bullet Physics:**

   ```bash
   sudo apt-get install libbullet-dev
   ```

2. **Install other dependencies:**

   ```bash
   sudo apt-get install libglfw3-dev libglm-dev libassimp-dev libfreetype6-dev
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

## Physics Integration

The project uses Bullet Physics engine for realistic car physics simulation including:

- Rigid body dynamics
- Collision detection
- Gravity simulation
- Force and torque-based controls
- Terrain interaction

### Key Physics Features

- **Mass-based simulation**: Car has proper mass (1000kg) and inertia
- **Force application**: Throttle and brake apply forces rather than direct velocity changes
- **Torque steering**: Steering applies rotational torque for realistic turning
- **Terrain snapping**: Car maintains contact with terrain surface

## Controls

- **W/Up Arrow** - Throttle
- **S/Down Arrow** - Brake/Reverse
- **A/Left Arrow** - Steer Left
- **D/Right Arrow** - Steer Right
- **ESC** - Exit

## Project Structure

```
.
├── src/
│   └── game_project/
│       ├── core/          # Car and control classes
│       ├── physics/       # Bullet physics integration
│       ├── scene/         # Scene and terrain management
│       └── input/         # Input handling
├── includes/              # Third-party headers
├── resources/             # Models, textures, etc.
└── build/                # Build directory
```

## Troubleshooting

### Header Conflicts

If you encounter compilation errors related to `NodeArray` or type conflicts between Assimp and Bullet:

- The project uses forward declarations to prevent header conflicts
- Bullet headers are only included in `.cpp` files, not in headers
- This keeps Assimp and Bullet headers separated

### CMake Can't Find Bullet

If CMake reports that it can't find Bullet:

```bash
# On macOS, check Bullet installation
brew list bullet

# If installed, you may need to help CMake find it
export BULLET_ROOT=/opt/homebrew/opt/bullet
```

## License

This project is for educational purposes.
