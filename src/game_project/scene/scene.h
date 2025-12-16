#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>

#include "../core/controls.h"
#include "../core/car.h"
#include <learnopengl/camera.h>
#include "Terrain.h"

// Forward declaration
class GameUI;

struct ModelInfo
{
  std::string path;
  std::string label;
};

class Scene
{
public:
  Scene();
  ~Scene();

  bool init(int scrWidth, int scrHeight, unsigned int terrainSeed = 0);

  bool showMenu(GLFWwindow *window, Shader &shader, GameUI &gameUI, Controls &controls, int &selectedIndex, int scrWidth, int scrHeight);

  bool showGameOver(GLFWwindow *window, Shader &shader, GameUI &gameUI, int finalScore, int scrWidth, int scrHeight);

  void renderScene(Shader &shader, Camera &camera, Car &car, int selectedIndex, int scrWidth, int scrHeight);

  void cleanup();

  void createCircularPlatform();

  // Access the scene's terrain for physics sampling
  Terrain &getTerrain() { return terrain; }

private:
  unsigned int groundVAO = 0;
  unsigned int groundVBO = 0;
  unsigned int groundEBO = 0;
  unsigned int groundTexture = 0;
  unsigned int backgroundTexture = 0;
  unsigned int bgVAO = 0;
  unsigned int bgVBO = 0;
  unsigned int bgEBO = 0;
  unsigned int platformVAO = 0;
  unsigned int platformVBO = 0;
  unsigned int platformEBO = 0;
  int platformIndexCount = 0;

  std::vector<ModelInfo> modelInfos;
  std::vector<std::string> modelPaths;
  std::vector<Model> models;

  Terrain terrain;
};
