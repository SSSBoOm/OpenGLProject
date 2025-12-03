#include "car.h"
#include <btBulletDynamicsCommon.h>

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
