#include "Terrain.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cstddef>

Terrain::Terrain() {}
Terrain::~Terrain() { cleanup(); }

static float sampleNoise(float x, float z)
{
  // Smooth random-looking terrain using multiple octaves of sine waves
  float v = 0.0f;
  v += 1.0f * std::sin(z * 0.05f) * std::cos(x * 0.03f);
  v += 0.5f * std::sin(z * 0.12f + 1.7f) * std::cos(x * 0.08f + 2.3f);
  v += 0.25f * std::sin(z * 0.23f + 3.1f) * std::cos(x * 0.15f + 4.2f);
  v += 0.125f * std::sin(z * 0.41f + 5.5f) * std::cos(x * 0.28f + 1.9f);
  return v;
}

bool Terrain::init(int w, int d, float s, float hscale)
{
  width = w;
  depth = d;
  scale = s;
  heightScale = hscale;

  heights.assign(width * depth, 0.0f);

  for (int iz = 0; iz < depth; ++iz)
  {
    for (int ix = 0; ix < width; ++ix)
    {
      float wx = (ix - width / 2) * scale;
      float wz = (iz - depth / 2) * scale;
      float h = sampleNoise(wx, wz) * heightScale * difficultyMultiplier;
      heights[iz * width + ix] = h;
    }
  }

  return generateMesh();
}

void Terrain::cleanup()
{
  if (EBO)
  {
    glDeleteBuffers(1, &EBO);
    EBO = 0;
  }
  if (VBO)
  {
    glDeleteBuffers(1, &VBO);
    VBO = 0;
  }
  if (VAO)
  {
    glDeleteVertexArrays(1, &VAO);
    VAO = 0;
  }
  indexCount = 0;
  heights.clear();
}

bool Terrain::generateMesh()
{
  if (width < 2 || depth < 2)
    return false;

  struct Vertex
  {
    float px, py, pz;
    float nx, ny, nz;
    float u, v;
  };
  std::vector<Vertex> verts;
  verts.reserve(width * depth);

  // helper to compute normal via central differences
  auto computeNormalAt = [&](int ix, int iz) -> glm::vec3
  {
    auto h = [&](int x, int z) -> float
    {
      x = std::clamp(x, 0, width - 1);
      z = std::clamp(z, 0, depth - 1);
      return heights[z * width + x];
    };
    float dx = h(ix + 1, iz) - h(ix - 1, iz);
    float dz = h(ix, iz + 1) - h(ix, iz - 1);
    glm::vec3 n = glm::normalize(glm::vec3(-dx, 2.0f, -dz));
    return n;
  };

  for (int iz = 0; iz < depth; ++iz)
  {
    for (int ix = 0; ix < width; ++ix)
    {
      float wx = (ix - width / 2) * scale + offsetX;
      float wz = (iz - depth / 2) * scale + offsetZ;
      float h = heights[iz * width + ix];
      glm::vec3 n = computeNormalAt(ix, iz);
      Vertex v;
      v.px = wx;
      v.py = h;
      v.pz = wz;
      v.nx = n.x;
      v.ny = n.y;
      v.nz = n.z;
      v.u = (float)ix / (float)(width - 1);
      v.v = (float)iz / (float)(depth - 1);
      verts.push_back(v);
    }
  }

  std::vector<unsigned int> indices;
  indices.reserve((width - 1) * (depth - 1) * 6);
  for (int iz = 0; iz < depth - 1; ++iz)
  {
    for (int ix = 0; ix < width - 1; ++ix)
    {
      unsigned int a = iz * width + ix;
      unsigned int b = iz * width + (ix + 1);
      unsigned int c = (iz + 1) * width + (ix + 1);
      unsigned int d = (iz + 1) * width + ix;
      // two triangles: a,b,c and a,c,d
      indices.push_back(a);
      indices.push_back(b);
      indices.push_back(c);
      indices.push_back(a);
      indices.push_back(c);
      indices.push_back(d);
    }
  }

  // upload to GPU
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, px));
  glEnableVertexAttribArray(0);
  // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, nx));
  glEnableVertexAttribArray(1);
  // texcoord
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, u));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  indexCount = static_cast<GLsizei>(indices.size());
  return true;
}

float Terrain::getHeight(float x, float z) const
{
  if (width < 2 || depth < 2)
    return 0.0f;
  // Adjust for terrain offset
  float fx = ((x - offsetX) / scale) + (width / 2.0f);
  float fz = ((z - offsetZ) / scale) + (depth / 2.0f);
  int ix = static_cast<int>(std::floor(fx));
  int iz = static_cast<int>(std::floor(fz));
  float tx = fx - ix;
  float tz = fz - iz;
  auto h = [&](int xidx, int zidx) -> float
  {
    xidx = std::clamp(xidx, 0, width - 1);
    zidx = std::clamp(zidx, 0, depth - 1);
    return heights[zidx * width + xidx];
  };
  float h00 = h(ix, iz);
  float h10 = h(ix + 1, iz);
  float h01 = h(ix, iz + 1);
  float h11 = h(ix + 1, iz + 1);
  float hx0 = h00 * (1 - tx) + h10 * tx;
  float hx1 = h01 * (1 - tx) + h11 * tx;
  float hval = hx0 * (1 - tz) + hx1 * tz;
  return hval;
}

glm::vec3 Terrain::getNormal(float x, float z) const
{
  const float EPS = 0.1f;
  float hL = getHeight(x - EPS, z);
  float hR = getHeight(x + EPS, z);
  float hD = getHeight(x, z - EPS);
  float hU = getHeight(x, z + EPS);
  glm::vec3 n = glm::normalize(glm::vec3(hL - hR, 2.0f * EPS, hD - hU));
  return n;
}

void Terrain::update(float playerX, float playerZ)
{
  // Regenerate terrain when player moves far from current center
  const float REGEN_DISTANCE = (width * scale) * 0.3f; // Regenerate when 30% away from center

  float distX = playerX - offsetX;
  float distZ = playerZ - offsetZ;

  bool needsRegeneration = false;

  // Check if player has moved far enough in X direction
  if (std::abs(distX) > REGEN_DISTANCE)
  {
    offsetX += (distX > 0 ? 1.0f : -1.0f) * REGEN_DISTANCE;
    needsRegeneration = true;
  }

  // Check if player has moved far enough in Z direction
  if (std::abs(distZ) > REGEN_DISTANCE)
  {
    offsetZ += (distZ > 0 ? 1.0f : -1.0f) * REGEN_DISTANCE;
    needsRegeneration = true;
  }

  if (needsRegeneration)
  {
    // Regenerate height data centered around new offset
    for (int iz = 0; iz < depth; ++iz)
    {
      for (int ix = 0; ix < width; ++ix)
      {
        float wx = (ix - width / 2) * scale + offsetX;
        float wz = (iz - depth / 2) * scale + offsetZ;
        float h = sampleNoise(wx, wz) * heightScale * difficultyMultiplier;
        heights[iz * width + ix] = h;
      }
    }

    // Rebuild the mesh with new heights
    generateMesh();
  }
}

void Terrain::render()
{
  if (!VAO)
    return;
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
