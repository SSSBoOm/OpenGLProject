#include "scene.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <stb_image.h>

#include "../input/input.h"

#include <iostream>

Scene::Scene() {}
Scene::~Scene() { cleanup(); }

bool Scene::init(int scrWidth, int scrHeight)
{
  // ground geometry
  float groundVertices[] = {
      -50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      50.0f, 0.0f, -50.0f, 0.0f, 1.0f, 0.0f, 10.0f, 0.0f,
      50.0f, 0.0f, 50.0f, 0.0f, 1.0f, 0.0f, 10.0f, 10.0f,
      -50.0f, 0.0f, 50.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f};

  unsigned int groundIndices[] = {0, 1, 2, 0, 2, 3};

  glGenVertexArrays(1, &groundVAO);
  glGenBuffers(1, &groundVBO);
  glGenBuffers(1, &groundEBO);

  glBindVertexArray(groundVAO);
  glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texcoord
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  // ground texture
  glGenTextures(1, &groundTexture);
  glBindTexture(GL_TEXTURE_2D, groundTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  std::string texturePath = FileSystem::getPath("resources/images/Concrete.jpg");
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
  if (data)
  {
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load ground texture: " << texturePath << std::endl;
  }
  stbi_image_free(data);

  // Background texture + quad (for menu)
  glGenTextures(1, &backgroundTexture);
  glBindTexture(GL_TEXTURE_2D, backgroundTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  std::string bgPath = FileSystem::getPath("resources/images/WhiteCloud.jpg");
  int bgW, bgH, bgC;
  stbi_set_flip_vertically_on_load(false);
  unsigned char *bgData = stbi_load(bgPath.c_str(), &bgW, &bgH, &bgC, 0);
  if (bgData)
  {
    GLenum bgFormat = (bgC == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, bgFormat, bgW, bgH, 0, bgFormat, GL_UNSIGNED_BYTE, bgData);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cerr << "Scene::init: Failed to load background texture: " << bgPath << std::endl;
  }
  stbi_image_free(bgData);

  float bgQuadVertices[] = {
      // positions        // texcoords
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
  unsigned int bgQuadIndices[] = {0, 1, 2, 0, 2, 3};

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

  // models (with labels)
  modelInfos = {
      {FileSystem::getPath("resources/objects/police_car/police_car.obj"), "Police Car"},
      {FileSystem::getPath("resources/objects/e30/e30.obj"), "BMW E30"},
      {FileSystem::getPath("resources/objects/pickup/pickup.obj"), "Pickup Truck"},
  };

  std::cerr << "Scene::init: modelInfos count = " << modelInfos.size() << std::endl;

  models.clear();
  models.reserve(modelInfos.size());
  for (const auto &mi : modelInfos)
  {
    std::cerr << " Scene::init: loading model: " << mi.path << " (label='" << mi.label << "')" << std::endl;
    models.emplace_back(mi.path);
  }

  std::cerr << "Scene::init: models loaded = " << models.size() << std::endl;
  // initialize procedural terrain (width, depth, scale, heightScale)
  if (!terrain.init(160, 1600, 1.0f, 3.5f))
  {
    std::cerr << "Scene::init: terrain initialization failed" << std::endl;
  }
  return true;
}

bool Scene::showMenu(GLFWwindow *window, Shader &shader, Controls &controls, int &selectedIndex, int scrWidth, int scrHeight)
{
  selectedIndex = 0;
  bool inMenu = true;
  double lastKeyTime = 0.0;
  const double keyDelay = 0.18; // seconds between selection changes

  while (inMenu && !glfwWindowShouldClose(window))
  {
    Input::handleEscape(window);
    Input::poll(window, controls);

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

    // Render selected model with orbit camera
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    shader.use();
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setMat4("view", glm::mat4(1.0f));
    shader.setMat4("projection", glm::mat4(1.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glBindVertexArray(bgVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    shader.use();
    float aspect = (float)scrWidth / (float)scrHeight;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    float orbitSpeed = 0.5f;
    float angle = static_cast<float>(glfwGetTime()) * orbitSpeed;
    float radius = 5.0f;
    glm::vec3 camPos = glm::vec3(sin(angle) * radius, 1.2f, cos(angle) * radius);
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.6f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.8f));
    model = glm::rotate(model, -angle * 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setMat4("model", model);

    if (models.empty())
    {
      std::cerr << "Scene::showMenu: models vector is empty, aborting menu." << std::endl;
      return false;
    }

    models[selectedIndex].Draw(shader);

    // ground
    glm::mat4 groundModel = glm::mat4(1.0f);
    shader.setMat4("model", groundModel);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glBindVertexArray(groundVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return !glfwWindowShouldClose(window);
}

void Scene::renderScene(Shader &shader, Camera &camera, Car &car, int selectedIndex, int scrWidth, int scrHeight)
{
  shader.use();
  glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);
  glm::mat4 view = camera.GetViewMatrix();
  shader.setMat4("projection", projection);
  shader.setMat4("view", view);

  glm::mat4 model = car.getModelMatrix();
  model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
  shader.setMat4("model", model);
  models[selectedIndex].Draw(shader);

  glm::mat4 groundModel = glm::mat4(1.0f);
  shader.setMat4("model", groundModel);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, groundTexture);
  // render procedural terrain in-game
  terrain.render();
}

void Scene::cleanup()
{
  if (groundVAO)
    glDeleteVertexArrays(1, &groundVAO);
  if (groundVBO)
    glDeleteBuffers(1, &groundVBO);
  if (groundEBO)
    glDeleteBuffers(1, &groundEBO);
  if (groundTexture)
    glDeleteTextures(1, &groundTexture);
  if (bgVAO)
    glDeleteVertexArrays(1, &bgVAO);
  if (bgVBO)
    glDeleteBuffers(1, &bgVBO);
  if (bgEBO)
    glDeleteBuffers(1, &bgEBO);
  if (backgroundTexture)
    glDeleteTextures(1, &backgroundTexture);
  terrain.cleanup();
}
