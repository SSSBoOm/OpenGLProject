#ifndef GAME_PROJECT_PHYSICS_H
#define GAME_PROJECT_PHYSICS_H

#include "../core/car.h"
#include "../core/controls.h"
#include "PhysicsWorld.h"
#include <learnopengl/camera.h>

// forward-declare Terrain to avoid circular includes
class Terrain;

namespace Physics
{
  void initializeCar(Car &car, PhysicsWorld &world, const glm::vec3 &startPos);
  void updateCar(Car &car, float dt, const Controls &c, PhysicsWorld &world, Terrain *terrain = nullptr);
  void updateCamera(const Car &car, Camera &cam);
}

#endif
