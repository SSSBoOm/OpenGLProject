#include "input.h"

namespace Input
{

  void poll(GLFWwindow *window, Controls &controls)
  {
    if (!window)
      return;
    controls.throttle = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    controls.brake = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    controls.boost = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
                     (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
  }

  void handleEscape(GLFWwindow *window)
  {
    if (!window)
      return;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
  }

}
