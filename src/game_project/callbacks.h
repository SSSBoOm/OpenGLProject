#ifndef GAME_PROJECT_CALLBACKS_H
#define GAME_PROJECT_CALLBACKS_H

#include <GLFW/glfw3.h>
// forward declare Camera to avoid pulling GL headers into this header
class Camera;

// Externs (defined in main.cpp)
extern Camera camera;
extern float lastX;
extern float lastY;
extern bool firstMouse;

// GLFW callbacks
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

#endif // GAME_PROJECT_CALLBACKS_H
