#!/bin/bash

# Build and Package Script for OpenGL Game
# This creates a portable distribution folder

echo "=== Building Game Project ==="

# 1. Clean and rebuild
cd build
cmake ..
make clean
make

# 2. Create distribution folder with same structure as bin/
echo "=== Creating Distribution Package ==="
cd ..
DIST_DIR="game_distribution"
rm -rf $DIST_DIR
mkdir -p $DIST_DIR/bin/game_project

# 3. Copy executable
echo "Copying executable..."
cp bin/game_project/game_project $DIST_DIR/bin/game_project/

# 4. Copy shaders
echo "Copying shaders..."
cp bin/game_project/*.vs $DIST_DIR/bin/game_project/ 2>/dev/null || true
cp bin/game_project/*.fs $DIST_DIR/bin/game_project/ 2>/dev/null || true

# 5. Copy resources with same relative structure
echo "Copying resources..."
cp -r resources $DIST_DIR/

# 6. Copy required libraries (if any dynamic ones)
echo "Copying libraries..."
# Check for dynamic library dependencies
if command -v otool &> /dev/null; then
    echo "Checking macOS dependencies..."
    otool -L bin/game_project/game_project | grep -v "/usr/lib" | grep -v "/System"
fi

# 7. Create README for distribution
cat > $DIST_DIR/README.txt << 'EOF'
=== Game Distribution ===

To run the game:
1. Open a terminal in this folder
2. Run: cd bin/game_project && ./game_project

The package structure matches the build structure:
- bin/game_project/game_project (executable)
- bin/game_project/*.vs, *.fs (shader files)
- resources/ (folder with fonts, images, objects)

Note: This build is for macOS (arm64). To run on another Mac, 
ensure OpenGL and required libraries are installed.

Required system libraries:
- OpenGL framework
- Cocoa framework
EOF

echo "=== Package Complete ==="
echo "Distribution folder: $DIST_DIR"
echo ""
echo "Folder structure:"
tree -L 2 $DIST_DIR 2>/dev/null || find $DIST_DIR -maxdepth 2 -print
echo ""
echo "To test, run: cd $DIST_DIR/bin/game_project && ./game_project"
