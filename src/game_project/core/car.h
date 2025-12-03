#ifndef GAME_PROJECT_CAR_H
#define GAME_PROJECT_CAR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Forward declaration to avoid header conflicts between Bullet and Assimp
class btRigidBody;
class btTransform;
class btVector3;
class btQuaternion;

class Car
{
public:
  glm::vec3 position{0.0f};
  float yaw = -90.0f;
  float pitch = 0.0f;
  float roll = 0.0f;
  float velocity = 0.0f;

  // Bullet physics rigid body
  btRigidBody *rigidBody = nullptr;

  // Sync position and rotation from Bullet rigid body
  void syncFromPhysics();

  glm::mat4 getModelMatrix() const
  {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
    return model;
  }
};

#endif
