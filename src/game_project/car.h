#ifndef GAME_PROJECT_CAR_H
#define GAME_PROJECT_CAR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Car {
public:
  glm::vec3 position{0.0f};
  float yaw = -90.0f;
  float velocity = 0.0f;

  glm::mat4 getModelMatrix() const
  {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    return model;
  }
};

#endif
