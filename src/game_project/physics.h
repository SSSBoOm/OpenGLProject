#ifndef GAME_PROJECT_PHYSICS_H
#define GAME_PROJECT_PHYSICS_H

#include "car.h"
#include "controls.h"
#include <learnopengl/camera.h>

namespace Physics {
  void updateCar(Car &car, float dt, const Controls &c);
  void updateCamera(const Car &car, Camera &cam);
}

#endif
