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
    constexpr float ACCEL_FORCE = 4000.0f;
    constexpr float BRAKE_FORCE = 2500.0f;
    constexpr float STEER_TORQUE = 300.0f;
    constexpr float MAX_SPEED = 32.0f;
    constexpr float MAX_SPEED_BOOST = 40.0f;
    constexpr float CAR_MASS = 750.0f;
    constexpr float MAX_REVERSE = 6.0f;
    constexpr float BOOST_FORCE_MULTIPLIER = 2.5f;
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
  btVector3 forward = basis * btVector3(-1, 0, 0); // Forward in local space (reversed)
  forward.setY(0);
  forward = forward.normalize();

  // Calculate speed along forward direction
  float forwardSpeed = velocity.dot(forward);

  // Apply throttle/brake forces
  if (c.throttle)
  {
    float maxSpeed = c.boost ? CFG::MAX_SPEED_BOOST : CFG::MAX_SPEED;
    float accelForce = c.boost ? CFG::ACCEL_FORCE * CFG::BOOST_FORCE_MULTIPLIER : CFG::ACCEL_FORCE;

    if (forwardSpeed < maxSpeed)
    {
      btVector3 force = forward * accelForce;
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

  // Calculate current speed magnitude
  float currentSpeed = velocity.length();
  const float FLIGHT_SPEED_THRESHOLD = 20.0f; // Speed required to take off
  const float LIFT_FORCE = 3500.0f;           // Upward force when flying at high speed
  const float TAKEOFF_BOOST = 2000.0f;        // Extra upward force to initiate takeoff
  const float AIR_DISTANCE = 1.0f;            // Distance above terrain to consider "airborne"

  // Four-wheel terrain contact system with realistic physics
  if (terrain != nullptr)
  {
    // Car dimensions (adjust these based on your car model size)
    const float WHEEL_BASE = 2.0f;   // Distance between front and rear axles
    const float TRACK_WIDTH = 1.5f;  // Distance between left and right wheels
    const float WHEEL_RADIUS = 0.4f; // Wheel size

    // Get car's current orientation
    float yawRad = glm::radians(car.yaw);
    glm::vec3 forward(cos(yawRad), 0.0f, sin(yawRad));
    glm::vec3 right(-forward.z, 0.0f, forward.x);

    // Calculate wheel positions in world space
    glm::vec3 frontLeft = car.position + forward * (WHEEL_BASE * 0.5f) + right * (TRACK_WIDTH * 0.5f);
    glm::vec3 frontRight = car.position + forward * (WHEEL_BASE * 0.5f) - right * (TRACK_WIDTH * 0.5f);
    glm::vec3 rearLeft = car.position - forward * (WHEEL_BASE * 0.5f) + right * (TRACK_WIDTH * 0.5f);
    glm::vec3 rearRight = car.position - forward * (WHEEL_BASE * 0.5f) - right * (TRACK_WIDTH * 0.5f);

    // Get terrain height at each wheel position
    float heightFL = terrain->getHeight(frontLeft.x, frontLeft.z);
    float heightFR = terrain->getHeight(frontRight.x, frontRight.z);
    float heightRL = terrain->getHeight(rearLeft.x, rearLeft.z);
    float heightRR = terrain->getHeight(rearRight.x, rearRight.z);

    // Calculate average terrain height
    float avgTerrainHeight = (heightFL + heightFR + heightRL + heightRR) * 0.25f;
    float distanceAboveTerrain = car.position.y - avgTerrainHeight;

    // Check if car should be flying
    bool isAirborne = distanceAboveTerrain > AIR_DISTANCE;
    bool wantsToFly = currentSpeed > FLIGHT_SPEED_THRESHOLD;

    // Apply lift force when going fast enough (both grounded and airborne)
    if (wantsToFly)
    {
      float speedRatio = (currentSpeed - FLIGHT_SPEED_THRESHOLD) / FLIGHT_SPEED_THRESHOLD;
      speedRatio = std::min(speedRatio, 2.0f); // Cap the lift multiplier

      // Base lift force
      btVector3 liftForce(0, LIFT_FORCE * speedRatio, 0);
      car.rigidBody->applyCentralForce(liftForce);

      // Extra boost when taking off from ground
      if (!isAirborne)
      {
        btVector3 takeoffForce(0, TAKEOFF_BOOST, 0);
        car.rigidBody->applyCentralForce(takeoffForce);
      }
    }

    // Ground alignment only when not flying fast
    if (!wantsToFly || distanceAboveTerrain < 0.2f)
    {
      // Ground mode - snap to terrain and align with surface
      // Calculate average height and pitch/roll from wheel heights
      float avgFront = (heightFL + heightFR) * 0.5f;
      float avgRear = (heightRL + heightRR) * 0.5f;
      float avgLeft = (heightFL + heightRL) * 0.5f;
      float avgRight = (heightFR + heightRR) * 0.5f;

      // Calculate pitch (front-rear tilt) and roll (left-right tilt)
      float targetPitch = -std::atan2(avgFront - avgRear, WHEEL_BASE);
      float targetRoll = std::atan2(avgLeft - avgRight, TRACK_WIDTH);

      // Get current orientation for smooth interpolation
      float currentPitch = glm::radians(car.pitch);
      float currentRoll = glm::radians(car.roll);

      // Smooth interpolation factor
      const float TERRAIN_ALIGN_SPEED = 6.0f;
      float lerpFactor = glm::clamp(TERRAIN_ALIGN_SPEED * dt, 0.0f, 1.0f);

      // Interpolate to target orientation
      float smoothPitch = currentPitch + (targetPitch - currentPitch) * lerpFactor;
      float smoothRoll = currentRoll + (targetRoll - currentRoll) * lerpFactor;

      // Average of all four wheels for car center height
      float targetHeight = avgTerrainHeight + WHEEL_RADIUS;

      // Only snap if car is below or close to terrain
      if (car.position.y <= targetHeight + 0.5f)
      {
        // Update car physics transform with new position and orientation
        btTransform newTrans;
        car.rigidBody->getMotionState()->getWorldTransform(newTrans);

        // Set new position
        newTrans.setOrigin(btVector3(car.position.x, targetHeight, car.position.z));

        // Create rotation quaternion from yaw, pitch, roll (using smoothed values)
        btQuaternion rotation;
        rotation.setEulerZYX(smoothRoll, glm::radians(car.yaw), smoothPitch);
        newTrans.setRotation(rotation);

        // Apply the transform
        car.rigidBody->setWorldTransform(newTrans);
        car.rigidBody->getMotionState()->setWorldTransform(newTrans);

        // Reset vertical velocity when landing
        btVector3 vel = car.rigidBody->getLinearVelocity();
        vel.setY(0);
        car.rigidBody->setLinearVelocity(vel);

        // Sync back to car
        car.syncFromPhysics();
      }
    }
  }
}

void Physics::updateCamera(const Car &car, Camera &cam)
{
  // Position camera to the right side of the car, slightly behind, and above
  const float SIDE_DIST = 15.0f;  // how far to the right of the car
  const float BACK_OFFSET = 1.0f; // small offset behind the car
  const float HEIGHT = 7.5f;      // camera height above car position
  const float PITCH_DEG = -20.0f; // desired camera pitch in degrees

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
