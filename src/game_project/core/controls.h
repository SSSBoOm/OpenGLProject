#ifndef GAME_PROJECT_CONTROLS_H
#define GAME_PROJECT_CONTROLS_H

// Simple container for input state
struct Controls
{
  bool throttle = false;
  bool brake = false;
  int steer = 0;      // -1 left, 0 none, +1 right
  bool boost = false; // shift key for boost
};

#endif // GAME_PROJECT_CONTROLS_H
