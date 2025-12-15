#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include "core/collectible.h"
#include "core/car.h"
#include "core/controls.h"
#include "core/callbacks.h"
#include "physics/physics.h"
#include "physics/PhysicsWorld.h"
#include "input/input.h"
#include "scene/scene.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

Controls controls;
Car car;
Collectibles collectibles;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  stbi_set_flip_vertically_on_load(true);
  glEnable(GL_DEPTH_TEST);
  Shader ourShader("1.model_loading.vs", "1.model_loading.fs");
  stbi_set_flip_vertically_on_load(false);

  // Initialize scene resources (ground, models, textures)
  Scene scene;
  scene.init(SCR_WIDTH, SCR_HEIGHT);

  int selectedIndex = 0;
  bool started = scene.showMenu(window, ourShader, controls, selectedIndex, SCR_WIDTH, SCR_HEIGHT);

  // If user closed window in menu, exit
  if (glfwWindowShouldClose(window) || !started)
  {
    glfwTerminate();
    return 0;
  }

  // Initialize Bullet Physics World
  PhysicsWorld physicsWorld;

  // Initialize car with physics
  Physics::initializeCar(car, physicsWorld, car.position);

  Model coinModel(FileSystem::getPath("resources/objects/coin/Coin.obj"));
  Model fuelModel(FileSystem::getPath("resources/objects/fuel/fuel.obj")); // TODO: Add fuel model
  // Model turboModel(FileSystem::getPath("resources/objects/turbo/Turbo.obj")); // TODO: Add turbo model
  
  collectibles.setModel(CollectibleType::COIN, &coinModel);
  collectibles.setModel(CollectibleType::COIN_RARE, &coinModel);
  collectibles.setModel(CollectibleType::FUEL, &fuelModel);
  // collectibles.setModel(CollectibleType::FUEL, &fuelModel);
  // collectibles.setModel(CollectibleType::TURBO, &turboModel);
  
  collectibles.init();
  // initial spawn: place coins ahead of the car along its current forward direction
  glm::vec3 initialForward = glm::vec3(std::cos(glm::radians(car.yaw)), 0.0f, std::sin(glm::radians(car.yaw)));
  // initial spawn: fewer coins spread further ahead
  collectibles.spawnAlongDirection(6, car.position, initialForward, &scene.getTerrain(), CollectibleType::COIN, 8.0f, 25.0f, 1.8f);

  // score tracking
  int score = 0;

  // Spawn control variables
  glm::vec3 lastSpawnPos = car.position;
  const float spawnMoveThreshold = 6.0f; // only spawn if player moved this far
  const int maxSpawnBatch = 6;           // max coins to spawn at once
  float lastSpawnTime = 0.0f;
  const float spawnCooldown = 1.0f;     // seconds between spawn batches

  // Main game loop: use chosen model
  while (!glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    Input::handleEscape(window);
    Input::poll(window, controls);

    Physics::updateCar(car, deltaTime, controls, physicsWorld, &scene.getTerrain());
    Physics::updateCamera(car, camera);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.renderScene(ourShader, camera, car, selectedIndex, SCR_WIDTH, SCR_HEIGHT);

    glm::vec3 forwardDir = glm::vec3(std::cos(glm::radians(car.yaw)), 0.0f, std::sin(glm::radians(car.yaw)));
    // Update collectible collection
    std::vector<CollectibleItem> collected;
    int newly = collectibles.updateCollect(car.position, 1.0f, forwardDir, car.velocity, collected);
    if (newly > 0) {
      score += newly;
      std::cout << "Collected " << collected.size() << " items. Remaining: " << collectibles.remaining() << "  Total Score: " << score << std::endl;
      // Handle different item types
      for (const auto& item : collected) {
        switch (item.type) {
          case CollectibleType::COIN:
          case CollectibleType::COIN_RARE:
            // Already added to score
            break;
          case CollectibleType::TURBO:
            std::cout << "  Turbo boost activated! +" << item.value << std::endl;
            // TODO: Apply turbo boost effect
            break;
          case CollectibleType::FUEL:
            std::cout << "  Fuel refilled! +" << item.value << std::endl;
            // TODO: Refill fuel
            break;
        }
      }
    }

    // Controlled respawn: spawn coins as the player moves forward in their facing direction
    float now = static_cast<float>(glfwGetTime());
    // compute how much the player moved forward along their facing direction since last spawn
    glm::vec3 delta = car.position - lastSpawnPos;
    glm::vec3 deltaXZ = glm::vec3(delta.x, 0.0f, delta.z);
    float forwardMoved = glm::dot(glm::normalize(glm::vec3(forwardDir.x, 0.0f, forwardDir.z)), glm::normalize(glm::length(deltaXZ) > 0.0001f ? deltaXZ : glm::vec3(0.0f)) ) * glm::length(deltaXZ);
    const float spawnForwardThreshold = 4.0f; // spawn when we've moved this far forward

    // Only spawn when moving forward and respecting cooldown
    if (forwardMoved > spawnForwardThreshold && (now - lastSpawnTime) > spawnCooldown) {
      // Check whether there are already coins in the forward sector; if so, don't spawn more
      const float checkMinF = 4.0f;
      const float checkMaxF = 20.0f;
      const float checkLat = 2.5f;
      if (!collectibles.hasItemsInDirection(car.position, forwardDir, checkMinF, checkMaxF, checkLat, 1, CollectibleType::COIN)) {
        // spawn in an elongated strip ahead of the car so positions align with movement direction
        // ensure coins spawn beyond immediate view by increasing minForward
        const float spawnMinF = 8.0f;
        const float spawnMaxF = 30.0f;
        const float spawnLat = 1.8f;
        
        // Spawn regular coins
        collectibles.spawnAlongDirection(maxSpawnBatch, car.position, forwardDir, &scene.getTerrain(), CollectibleType::COIN, spawnMinF, spawnMaxF, spawnLat);
        
        // 30% chance to spawn 1 rare coin
        if ((std::rand() % 100) < 30) {
          collectibles.spawnAlongDirection(1, car.position, forwardDir, &scene.getTerrain(), CollectibleType::COIN_RARE, spawnMinF, spawnMaxF, spawnLat);
        }
        
        // 10% chance to spawn 1 fuel
        if ((std::rand() % 100) < 100) {
          collectibles.spawnAlongDirection(1, car.position, forwardDir, &scene.getTerrain(), CollectibleType::FUEL, spawnMinF, spawnMaxF, spawnLat);
        }
        
        lastSpawnPos = car.position;
        lastSpawnTime = now;
      }
    }

    collectibles.draw(ourShader, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  scene.cleanup();
  glfwTerminate();
  return 0;
}