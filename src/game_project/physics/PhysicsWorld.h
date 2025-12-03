#ifndef GAME_PROJECT_PHYSICS_WORLD_H
#define GAME_PROJECT_PHYSICS_WORLD_H

#include <memory>

// Forward declarations to avoid header conflicts between Bullet and Assimp
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btVector3;
class btTriangleMesh;

class PhysicsWorld
{
public:
  PhysicsWorld();
  ~PhysicsWorld();

  void stepSimulation(float deltaTime);
  btDiscreteDynamicsWorld *getDynamicsWorld();

  // Helper to create a rigid body for the car
  btRigidBody *createCarRigidBody(const btVector3 &position, float mass);

  // Helper to create a static terrain collision shape
  btRigidBody *createTerrainBody(btTriangleMesh *terrainMesh);

private:
  std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
  std::unique_ptr<btCollisionDispatcher> dispatcher;
  std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
  std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
  std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
};

#endif
