#include "physics.h"
#include "../scene/Terrain.h"
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

namespace
{
  namespace CFG
  {
    constexpr float ACCEL_FORCE = 2000.0f;
    constexpr float BRAKE_FORCE = 3000.0f;
    constexpr float STEER_TORQUE = 300.0f;
    constexpr float MAX_SPEED = 12.0f;
    constexpr float MAX_REVERSE = 6.0f;
    constexpr float CAR_MASS = 1000.0f;
  }
}

void Physics::initializeCar(Car &car, PhysicsWorld &world, const glm::vec3 &startPos)
{
  // Create the car's rigid body in Bullet
  btVector3 pos(startPos.x, startPos.y + 2.0f, startPos.z); // Start slightly above ground
  car.rigidBody = world.createCarRigidBody(pos, CFG::CAR_MASS);

  // Set initial orientation
  btTransform trans;
  car.rigidBody->getMotionState()->getWorldTransform(trans);
  btQuaternion rotation;
  rotation.setEulerZYX(0, glm::radians(car.yaw), 0);
  trans.setRotation(rotation);
  car.rigidBody->getMotionState()->setWorldTransform(trans);
  car.rigidBody->setWorldTransform(trans);

  // Initial sync
  car.syncFromPhysics();
}

void Physics::updateCar(Car &car, float dt, const Controls &c, PhysicsWorld &world, Terrain *terrain)
{
  if (!car.rigidBody)
  {
    return;
  }

  // Get current transform and velocity
  btTransform trans;
  car.rigidBody->getMotionState()->getWorldTransform(trans);
  btVector3 velocity = car.rigidBody->getLinearVelocity();

  // Get forward direction from current orientation
  btMatrix3x3 basis = trans.getBasis();
  btVector3 forward = basis * btVector3(1, 0, 0); // Forward in local space
  forward.setY(0);
  forward = forward.normalize();

  // Calculate speed along forward direction
  float forwardSpeed = velocity.dot(forward);

  // Apply throttle/brake forces
  if (c.throttle)
  {
    if (forwardSpeed < CFG::MAX_SPEED)
    {
      btVector3 force = forward * CFG::ACCEL_FORCE;
      car.rigidBody->applyCentralForce(force);
    }
  }

  if (c.brake)
  {
    if (forwardSpeed > -CFG::MAX_REVERSE)
    {
      btVector3 force = forward * -CFG::BRAKE_FORCE;
      car.rigidBody->applyCentralForce(force);
    }
  }

  // Apply steering torque
  if (c.steer < 0) // left
  {
    btVector3 torque(0, CFG::STEER_TORQUE, 0);
    car.rigidBody->applyTorque(torque);
  }
  if (c.steer > 0) // right
  {
    btVector3 torque(0, -CFG::STEER_TORQUE, 0);
    car.rigidBody->applyTorque(torque);
  }

  // Step the physics simulation
  world.stepSimulation(dt);

  // Sync car state from physics
  car.syncFromPhysics();

  // Optional: Snap to terrain height if terrain exists
  if (terrain != nullptr)
  {
    float terrainHeight = terrain->getHeight(car.position.x, car.position.z);
    const float CAR_BASE_HEIGHT = 0.5f;

    // If car is below terrain, push it up
    if (car.position.y < terrainHeight + CAR_BASE_HEIGHT)
    {
      btTransform newTrans;
      car.rigidBody->getMotionState()->getWorldTransform(newTrans);
      newTrans.setOrigin(btVector3(car.position.x, terrainHeight + CAR_BASE_HEIGHT, car.position.z));
      car.rigidBody->setWorldTransform(newTrans);
      car.rigidBody->getMotionState()->setWorldTransform(newTrans);

      // Reset vertical velocity
      btVector3 vel = car.rigidBody->getLinearVelocity();
      vel.setY(0);
      car.rigidBody->setLinearVelocity(vel);

      car.syncFromPhysics();
    }
  }
}

void Physics::updateCamera(const Car &car, Camera &cam)
{
  // Position camera to the right side of the car, slightly behind, and above
  const float SIDE_DIST = 10.0f;  // how far to the right of the car
  const float BACK_OFFSET = 1.0f; // small offset behind the car
  const float HEIGHT = 2.5f;      // camera height above car position
  const float PITCH_DEG = -15.0f; // desired camera pitch in degrees

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
