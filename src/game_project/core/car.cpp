#include "car.h"
#include <btBulletDynamicsCommon.h>
#include <algorithm>

void Car::syncFromPhysics()
{
  if (rigidBody)
  {
    btTransform trans;
    rigidBody->getMotionState()->getWorldTransform(trans);

    // Update position
    btVector3 pos = trans.getOrigin();
    position = glm::vec3(pos.x(), pos.y(), pos.z());

    // Update rotation (yaw, pitch, roll)
    btQuaternion rot = trans.getRotation();
    btScalar yawRad, pitchRad, rollRad;
    rot.getEulerZYX(rollRad, yawRad, pitchRad);

    yaw = glm::degrees(yawRad);
    pitch = glm::degrees(pitchRad);
    roll = glm::degrees(rollRad);

    // Calculate velocity magnitude
    btVector3 vel = rigidBody->getLinearVelocity();
    velocity = vel.length();
  }
}

void Car::updateFuel(float deltaTime, bool isMoving)
{
  if (isMoving && fuel > 0.0f)
  {
    // Deplete fuel based on movement
    fuel -= fuelDepletionRate * deltaTime;
    fuel = std::max(0.0f, fuel);
  }
}

void Car::addFuel(float amount)
{
  fuel += amount;
  fuel = std::min(fuel, maxFuel);
}

void Car::addTurbo(float amount)
{
  turbo += amount;
  turbo = std::min(turbo, maxTurbo);
}

void Car::useTurbo(float deltaTime)
{
  if (turbo > 0.0f)
  {
    turbo -= turboDepletionRate * deltaTime;
    turbo = std::max(0.0f, turbo);
  }
}
