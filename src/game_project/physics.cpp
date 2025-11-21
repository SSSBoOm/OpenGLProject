#include "physics.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {
namespace CFG {
  constexpr float ACCEL = 6.0f;
  constexpr float BRAKE = 10.0f;
  constexpr float DRAG = 1.5f;
  constexpr float MAX_SPEED = 25.0f;
  constexpr float MAX_REVERSE = 8.0f;
  constexpr float STEER_SPEED = 90.0f;
}
}

void Physics::updateCar(Car &car, float dt, const Controls &c)
{
  float accel = 0.0f;
  if (c.throttle)
    accel += CFG::ACCEL;
  if (c.brake)
    accel -= CFG::BRAKE;

  if (!c.throttle && !c.brake)
    accel += -CFG::DRAG * car.velocity;

  car.velocity += accel * dt;
  if (car.velocity > CFG::MAX_SPEED)
    car.velocity = CFG::MAX_SPEED;
  if (car.velocity < -CFG::MAX_REVERSE)
    car.velocity = -CFG::MAX_REVERSE;

  float speedFactor = CFG::MAX_SPEED > 0.0f ? glm::clamp(glm::abs(car.velocity) / CFG::MAX_SPEED, 0.0f, 1.0f) : 0.0f;
  float steerDelta = (float)c.steer * CFG::STEER_SPEED * speedFactor * dt;
  if (car.velocity < 0.0f)
    steerDelta = -steerDelta;
  car.yaw -= steerDelta;

  float yawRad = glm::radians(car.yaw);
  glm::vec3 forwardVec = glm::vec3(cos(yawRad), 0.0f, sin(yawRad));
  car.position += forwardVec * car.velocity * dt;
}

void Physics::updateCamera(const Car &car, Camera &cam)
{
  const float FOLLOW_DIST = 6.0f;
  const float FOLLOW_HEIGHT = 2.5f;
  float yawRad = glm::radians(car.yaw);
  glm::vec3 forwardVec = glm::vec3(cos(yawRad), 0.0f, sin(yawRad));
  glm::vec3 desiredCamPos = car.position - forwardVec * FOLLOW_DIST + glm::vec3(0.0f, FOLLOW_HEIGHT, 0.0f);
  cam.Position = desiredCamPos;
  cam.Front = glm::normalize((car.position + glm::vec3(0.0f, 1.0f, 0.0f)) - cam.Position);
  cam.Right = glm::normalize(glm::cross(cam.Front, cam.WorldUp));
  cam.Up = glm::normalize(glm::cross(cam.Right, cam.Front));
}
