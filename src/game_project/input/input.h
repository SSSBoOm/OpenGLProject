#ifndef GAME_PROJECT_INPUT_H
#define GAME_PROJECT_INPUT_H

#include <GLFW/glfw3.h>
#include "../core/controls.h"

namespace Input {
  // Poll keyboard and populate Controls struct
  void poll(GLFWwindow* window, Controls &controls);
  // Handle escape key (close window)
  void handleEscape(GLFWwindow* window);
}

#endif // GAME_PROJECT_INPUT_H
