#include "input.h"

namespace Input {

    void poll(GLFWwindow* window, Controls &controls)
    {
        if (!window) return;
        controls.throttle = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
        controls.brake = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
        // steering
        int steer = 0;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) steer -= 1;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) steer += 1;
        controls.steer = steer;
    }

    void handleEscape(GLFWwindow* window)
    {
        if (!window) return;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

}
