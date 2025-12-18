# Game Deployment Guide

## Quick Start - Build and Package

### For macOS/Linux:

```bash
# Make script executable (first time only)
chmod +x package.sh

# Build and create distribution
./package.sh
```

This creates a `game_distribution/` folder with everything needed to run on another computer.

---

## Manual Build Process

### Step 1: Configure with Relative Paths

The project is now configured to use relative paths. The executable will look for resources relative to its location:

```
game_project              # Executable
resources/                # Resources folder
  ├── fonts/
  ├── images/
  └── objects/
*.vs, *.fs               # Shader files
```

### Step 2: Build the Project

```bash
cd build
cmake ..
make
```

### Step 3: Create Distribution Folder

```bash
# Create distribution directory
mkdir -p game_distribution

# Copy executable
cp bin/game_project/game_project game_distribution/

# Copy shaders
cp bin/game_project/*.vs bin/game_project/*.fs game_distribution/

# Copy resources
cp -r resources game_distribution/
```

---

## Distribution Structure

Your portable folder should look like this:

```
game_distribution/
├── game_project          # Executable
├── resources/
│   ├── fonts/
│   ├── images/
│   └── objects/
├── 1.model_loading.vs
├── 1.model_loading.fs
├── icon.vs
├── icon.fs
├── text.vs
├── text.fs
├── ui.vs
├── ui.fs
└── README.txt
```

---

## Running on Another Computer

### macOS:

1. Copy the entire `game_distribution/` folder to the target Mac
2. Open Terminal in that folder
3. Run: `./game_project`

**Note:** The target Mac needs:

- macOS with arm64 architecture (Apple Silicon)
- System OpenGL frameworks (built-in)

### For Other Platforms:

#### Windows:

You'll need to:

1. Rebuild on Windows with Visual Studio
2. Copy required DLL files (GLFW, ASSIMP, etc.)
3. Update CMake to use Windows paths

#### Linux:

You'll need to:

1. Rebuild on Linux
2. Ensure shared libraries are included or statically linked
3. Consider using AppImage for better portability

---

## Static Linking (Advanced)

For maximum portability, consider static linking:

Edit `CMakeLists.txt`:

```cmake
# Add these compiler flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
```

---

## Troubleshooting

### "resources not found" error:

- Ensure `resources/` folder is in the same directory as executable
- Check file permissions: `chmod -R 755 resources/`

### Library errors on target machine:

```bash
# Check dependencies
otool -L game_project  # macOS
ldd game_project       # Linux

# Install missing dependencies via Homebrew (macOS)
brew install glfw assimp freetype bullet
```

### Executable won't run:

```bash
# Make sure it's executable
chmod +x game_project
```

---

## Environment Variable Override

You can also set `LOGL_ROOT_PATH` environment variable:

```bash
export LOGL_ROOT_PATH=/path/to/project
./game_project
```

This overrides the compiled path, useful for testing.
