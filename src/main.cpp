#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, true);
  }
  // Use Tab to reset the window title to "OpenGL"
  static std::string titleBuffer;
  static bool keyStates[GLFW_KEY_LAST + 1] = {false};

  for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key)
  {
    int state = glfwGetKey(window, key);
    if (state == GLFW_PRESS && !keyStates[key])
    {
      if (key == GLFW_KEY_TAB)
      {
        titleBuffer.clear();
        glfwSetWindowTitle(window, "OpenGL");
      }
      else
      {
        // Append the key code as a string to the title buffer
        titleBuffer += std::to_string(key);
        glfwSetWindowTitle(window, titleBuffer.c_str());
      }
      keyStates[key] = true;
    }
    else if (state == GLFW_RELEASE)
    {
      keyStates[key] = false;
    }
  }
}

int main()
{
  // glfw: initialize and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // glfw window creation
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);

  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // render loop
  while (!glfwWindowShouldClose(window))
  {
    // input
    processInput(window);

    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // check and call events and swap the buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}