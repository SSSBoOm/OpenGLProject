#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader;

class Terrain
{
public:
  Terrain();
  ~Terrain();

  // Initialize terrain mesh. width/depth are grid counts, scale is spacing, heightScale multiplies the generated height
  bool init(int width = 128, int depth = 128, float scale = 1.0f, float heightScale = 2.5f, unsigned int seed = 0);
  void cleanup();

  // Render terrain (uses currently bound shader; shader must accept 'model')
  void render();

  // Update terrain position for infinite generation based on player position
  void update(float playerX, float playerZ);

  // Set difficulty multiplier (1.0 = normal, higher = steeper terrain)
  void setDifficultyMultiplier(float multiplier) { difficultyMultiplier = multiplier; }

  // Sample terrain height at world (x,z)
  float getHeight(float x, float z) const;
  // Estimated normal at world (x,z)
  glm::vec3 getNormal(float x, float z) const;

private:
  bool generateMesh();

  int width = 0;
  int depth = 0;
  float scale = 1.0f;
  float heightScale = 1.0f;
  float difficultyMultiplier = 1.0f; // Increases terrain steepness over distance
  unsigned int terrainSeed = 0;      // Seed for procedural terrain generation

  // Offset for infinite terrain generation
  float offsetX = 0.0f;
  float offsetZ = 0.0f;

  std::vector<float> heights; // size width*depth

  GLuint VAO = 0;
  GLuint VBO = 0;
  GLuint EBO = 0;
  GLsizei indexCount = 0;
};
