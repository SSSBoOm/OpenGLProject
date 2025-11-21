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
#include "core/callbacks.h"
#include "physics/physics.h"
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

  // Main game loop: use chosen model
  while (!glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    Input::handleEscape(window);
    Input::poll(window, controls);

    Physics::updateCar(car, deltaTime, controls, &scene.getTerrain());
    Physics::updateCamera(car, camera);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.renderScene(ourShader, camera, car, selectedIndex, SCR_WIDTH, SCR_HEIGHT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  scene.cleanup();
  glfwTerminate();
  return 0;
}