#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include "core/car.h"
#include "core/controls.h"
#include "physics/physics.h"

#include <iostream>

#include "core/callbacks.h"
#include "input/input.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

Controls controls;
Car car;

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

  float groundVertices[] = {
      -50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, 10.0f, 0.0f,
      50.0f, 0.0f, 50.0f, 0.0f, 1.0f, 0.0f, 10.0f, 10.0f,
      -50.0f, 0.0f, 50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f};

  unsigned int groundIndices[] = {
      0, 1, 2,
      0, 2, 3};

  unsigned int groundVAO, groundVBO, groundEBO;
  glGenVertexArrays(1, &groundVAO);
  glGenBuffers(1, &groundVBO);
  glGenBuffers(1, &groundEBO);

  glBindVertexArray(groundVAO);
  glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texture coord attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  // Load ground texture
  unsigned int groundTexture;
  glGenTextures(1, &groundTexture);
  glBindTexture(GL_TEXTURE_2D, groundTexture);
  // Set texture wrapping and filtering options
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Load and generate texture
  int width, height, nrChannels;
  std::string texturePath = FileSystem::getPath("resources/images/Concrete.jpg");
  unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load ground texture" << std::endl;
  }
  stbi_image_free(data);

  // Load background texture for menu
  unsigned int backgroundTexture;
  glGenTextures(1, &backgroundTexture);
  glBindTexture(GL_TEXTURE_2D, backgroundTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  std::string bgPath = FileSystem::getPath("resources/images/WhiteCloud.jpg");
  int bgWidth, bgHeight, bgChannels;
  stbi_set_flip_vertically_on_load(false);
  unsigned char *bgData = stbi_load(bgPath.c_str(), &bgWidth, &bgHeight, &bgChannels, 0);
  if (bgData)
  {
    GLenum bgFormat = (bgChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgWidth, bgHeight, 0, bgFormat, GL_UNSIGNED_BYTE, bgData);
    stbi_image_free(bgData);
  }
  stbi_set_flip_vertically_on_load(true);

  // Background quad for menu
  float bgQuadVertices[] = {
      // positions        // texcoords
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

  unsigned int bgQuadIndices[] = {0, 1, 2, 0, 2, 3};

  unsigned int bgVAO, bgVBO, bgEBO;
  glGenVertexArrays(1, &bgVAO);
  glGenBuffers(1, &bgVBO);
  glGenBuffers(1, &bgEBO);

  glBindVertexArray(bgVAO);
  glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(bgQuadVertices), bgQuadVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bgQuadIndices), bgQuadIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  // Available model paths (relative to project root via FileSystem)
  struct ModelInfo
  {
    std::string path;
    std::string label;
  };

  std::vector<ModelInfo> modelInfos = {
      {FileSystem::getPath("resources/objects/police_car/police_car.obj"), "Police Car"},
      {FileSystem::getPath("resources/objects/e30/e30.obj"), "BMW E30"},
      {FileSystem::getPath("resources/objects/pickup/pickup.obj"), "Pickup Truck"},
  };

  std::vector<std::string> modelPaths;
  modelPaths.reserve(modelInfos.size());
  for (const auto &info : modelInfos)
  {
    modelPaths.push_back(info.path);
  }

  // Load all models for the menu (keeps them in memory for quick switching)
  std::vector<Model> models;
  stbi_set_flip_vertically_on_load(false);
  models.reserve(modelPaths.size());
  for (const auto &p : modelPaths)
  {
    models.emplace_back(p);
  }

  // Menu selection index
  int selectedIndex = 0;
  bool inMenu = true;
  double lastKeyTime = 0.0;
  const double keyDelay = 0.18; // seconds between selection changes

  // First: simple menu loop for selecting model
  while (inMenu && !glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    Input::handleEscape(window);
    // Poll driving inputs only when in-game; for menu we'll just check keys directly
    Input::poll(window, controls);

    // Simple key debounce based on glfw time
    double t = glfwGetTime();
    if (t - lastKeyTime > keyDelay)
    {
      if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
      {
        selectedIndex = (selectedIndex + 1) % static_cast<int>(models.size());
        lastKeyTime = t;
        std::string title = std::string("Select Model - ") + modelInfos[selectedIndex].label;
        glfwSetWindowTitle(window, title.c_str());
      }
      if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
      {
        selectedIndex = (selectedIndex - 1 + static_cast<int>(models.size())) % static_cast<int>(models.size());
        lastKeyTime = t;
        std::string title = std::string("Select Model - ") + modelInfos[selectedIndex].label;
        glfwSetWindowTitle(window, title.c_str());
      }
      if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      {
        inMenu = false; // start the game
        lastKeyTime = t;
        break;
      }
    }

    // Render the currently selected model in the center with a slow orbit/dolly
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw background image as fullscreen quad
    glDisable(GL_DEPTH_TEST);
    ourShader.use();
    ourShader.setMat4("model", glm::mat4(1.0f));
    ourShader.setMat4("view", glm::mat4(1.0f));
    ourShader.setMat4("projection", glm::mat4(1.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glBindVertexArray(bgVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    ourShader.use();
    float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    // Orbit camera around origin (where the model will be placed)
    float orbitSpeed = 0.5f; // angular speed multiplier
    float angle = static_cast<float>(glfwGetTime()) * orbitSpeed;
    float radius = 5.0f;
    glm::vec3 camPos = glm::vec3(sin(angle) * radius, 1.2f, cos(angle) * radius);
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.6f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);

    // Place and scale model so it fits in view
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.8f));
    // slow model rotation for a pleasing effect as well
    model = glm::rotate(model, -angle * 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
    ourShader.setMat4("model", model);

    models[selectedIndex].Draw(ourShader);

    // Draw ground plane with texture
    glm::mat4 groundModel = glm::mat4(1.0f);
    ourShader.setMat4("model", groundModel);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glBindVertexArray(groundVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // If user closed window in menu, exit
  if (glfwWindowShouldClose(window))
  {
    glfwTerminate();
    return 0;
  }

  // Main game loop: use chosen model
  while (!glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    Input::handleEscape(window);
    Input::poll(window, controls);

    Physics::updateCar(car, deltaTime, controls);
    Physics::updateCamera(car, camera);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader.use();
    glm::mat4 projection2 = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view2 = camera.GetViewMatrix();
    ourShader.setMat4("projection", projection2);
    ourShader.setMat4("view", view2);
    glm::mat4 model2 = car.getModelMatrix();
    model2 = glm::scale(model2, glm::vec3(1.0f, 1.0f, 1.0f));
    ourShader.setMat4("model", model2);
    models[selectedIndex].Draw(ourShader);

    // Draw ground plane with texture
    glm::mat4 groundModel2 = glm::mat4(1.0f);
    ourShader.setMat4("model", groundModel2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glBindVertexArray(groundVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  glDeleteVertexArrays(1, &groundVAO);
  glDeleteBuffers(1, &groundVBO);
  glDeleteBuffers(1, &groundEBO);
  glDeleteTextures(1, &groundTexture);
  glDeleteVertexArrays(1, &bgVAO);
  glDeleteBuffers(1, &bgVBO);
  glDeleteBuffers(1, &bgEBO);
  glDeleteTextures(1, &backgroundTexture);
  glfwTerminate();
  return 0;
}