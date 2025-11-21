#include "physics.h"
#include "../scene/Terrain.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

namespace {
namespace CFG {
  constexpr float ACCEL = 3.0f;
  constexpr float BRAKE = 10.0f;
  constexpr float DRAG = 1.5f;
  constexpr float MAX_SPEED = 12.0f;
  constexpr float MAX_REVERSE = 6.0f;
  constexpr float STEER_SPEED = 90.0f;
}
}

void Physics::updateCar(Car &car, float dt, const Controls &c, Terrain *terrain)
{
  float accel = 0.0f;
  if (c.throttle)
    accel += CFG::ACCEL;
  if (c.brake)
    accel -= CFG::BRAKE;

  if (!c.throttle && !c.brake)
    accel += -CFG::DRAG * car.velocity;
  // Update velocity from throttle/brake/drag
  car.velocity += accel * dt;
  if (car.velocity > CFG::MAX_SPEED)
    car.velocity = CFG::MAX_SPEED;
  if (car.velocity < -CFG::MAX_REVERSE)
    car.velocity = -CFG::MAX_REVERSE;

  float speedFactor = CFG::MAX_SPEED > 0.0f ? glm::clamp(glm::abs(car.velocity) / CFG::MAX_SPEED, 0.0f, 1.0f) : 0.0f;

  // Disable steering: keep car orientation fixed so it only moves forward/backward
  float yawRad = glm::radians(car.yaw);
  glm::vec3 forwardVec = glm::vec3(cos(yawRad), 0.0f, sin(yawRad));

  // If we have a terrain, add gravity component along the slope and update pitch/roll
  if (terrain != nullptr)
  {
    const float EPS = 0.5f; // sample offset for slope approximation
    const float GRAVITY = 9.81f;

    float h_center = terrain->getHeight(car.position.x, car.position.z);
    float h_ahead = terrain->getHeight(car.position.x + forwardVec.x * EPS, car.position.z + forwardVec.z * EPS);
    float h_back = terrain->getHeight(car.position.x - forwardVec.x * EPS, car.position.z - forwardVec.z * EPS);
    float slope = (h_ahead - h_back) / (2.0f * EPS); // dh/ds along forward direction

    // gravity acceleration along slope (tunable multiplier)
    const float SLOPE_SCALE = 6.0f; // tuned scale to feel good in gameplay
    float slopeAccel = -GRAVITY * slope * SLOPE_SCALE;
    car.velocity += slopeAccel * dt;

    // lateral slope -> roll
    glm::vec3 rightVec = glm::vec3(-forwardVec.z, 0.0f, forwardVec.x);
    float h_left = terrain->getHeight(car.position.x + rightVec.x * EPS, car.position.z + rightVec.z * EPS);
    float h_right = terrain->getHeight(car.position.x - rightVec.x * EPS, car.position.z - rightVec.z * EPS);
    float latSlope = (h_right - h_left) / (2.0f * EPS);

    // Update position horizontally
    car.position += forwardVec * car.velocity * dt;

    // Snap vertical position to terrain plus an offset (car half-height)
    const float CAR_BASE_HEIGHT = 0.5f;
    float groundH = terrain->getHeight(car.position.x, car.position.z);
    car.position.y = groundH + CAR_BASE_HEIGHT;

    // update pitch and roll from slopes (convert to degrees)
    car.pitch = glm::degrees(std::atan(slope));
    car.roll = glm::degrees(std::atan(latSlope));
  }
  else
  {
    // No terrain: simple horizontal move
    car.position += forwardVec * car.velocity * dt;
    car.pitch = 0.0f;
    car.roll = 0.0f;
  }
}

void Physics::updateCamera(const Car &car, Camera &cam)
{
  // Position camera to the right side of the car, slightly behind, and above
  const float SIDE_DIST = 10.0f;     // how far to the right of the car
  const float BACK_OFFSET = 1.0f;   // small offset behind the car
  const float HEIGHT = 2.5f;        // camera height above car position
  const float PITCH_DEG = -15.0f;   // desired camera pitch in degrees

  float yawRad = glm::radians(car.yaw);
  glm::vec3 forwardVec = glm::vec3(cos(yawRad), 0.0f, sin(yawRad));
  // right vector from forward and world-up
  glm::vec3 rightVec = glm::vec3(-forwardVec.z, 0.0f, forwardVec.x);

  glm::vec3 desiredCamPos = car.position + rightVec * SIDE_DIST - forwardVec * BACK_OFFSET + glm::vec3(0.0f, HEIGHT, 0.0f);
  cam.Position = desiredCamPos;

  // Compute horizontal yaw to look roughly toward the car, but enforce the requested pitch
  float lookX = car.position.x - cam.Position.x;
  float lookZ = car.position.z - cam.Position.z;
  float yawDeg = glm::degrees(std::atan2(lookZ, lookX));
  cam.Yaw = yawDeg;
  cam.Pitch = PITCH_DEG;

  // Recompute Front/Right/Up using the camera's Euler angles (same math as Camera::updateCameraVectors)
  glm::vec3 front;
  front.x = cos(glm::radians(cam.Yaw)) * cos(glm::radians(cam.Pitch));
  front.y = sin(glm::radians(cam.Pitch));
  front.z = sin(glm::radians(cam.Yaw)) * cos(glm::radians(cam.Pitch));
  cam.Front = glm::normalize(front);
  cam.Right = glm::normalize(glm::cross(cam.Front, cam.WorldUp));
  cam.Up = glm::normalize(glm::cross(cam.Right, cam.Front));
}
