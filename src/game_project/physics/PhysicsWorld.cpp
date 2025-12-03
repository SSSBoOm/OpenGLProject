#include "PhysicsWorld.h"
#include <btBulletDynamicsCommon.h>

btDiscreteDynamicsWorld *PhysicsWorld::getDynamicsWorld()
{
  return dynamicsWorld.get();
}

PhysicsWorld::PhysicsWorld()
{
  // Bullet physics initialization
  collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
  dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());
  overlappingPairCache = std::make_unique<btDbvtBroadphase>();
  solver = std::make_unique<btSequentialImpulseConstraintSolver>();

  dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
      dispatcher.get(),
      overlappingPairCache.get(),
      solver.get(),
      collisionConfiguration.get());

  // Set gravity
  dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
}

PhysicsWorld::~PhysicsWorld()
{
  // Cleanup is handled by unique_ptr destructors
}

void PhysicsWorld::stepSimulation(float deltaTime)
{
  dynamicsWorld->stepSimulation(deltaTime, 10);
}

btRigidBody *PhysicsWorld::createCarRigidBody(const btVector3 &position, float mass)
{
  // Create a box collision shape for the car
  btCollisionShape *carShape = new btBoxShape(btVector3(1.0f, 0.5f, 2.0f));

  btTransform carTransform;
  carTransform.setIdentity();
  carTransform.setOrigin(position);

  btVector3 localInertia(0, 0, 0);
  if (mass != 0.0f)
  {
    carShape->calculateLocalInertia(mass, localInertia);
  }

  btDefaultMotionState *motionState = new btDefaultMotionState(carTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, carShape, localInertia);

  // Adjust physics properties for car-like behavior
  rbInfo.m_friction = 0.8f;
  rbInfo.m_restitution = 0.1f;
  rbInfo.m_linearDamping = 0.3f;
  rbInfo.m_angularDamping = 0.5f;

  btRigidBody *body = new btRigidBody(rbInfo);

  // Prevent rotation on X and Z axes (only allow Y-axis rotation for steering)
  body->setAngularFactor(btVector3(0, 1, 0));

  dynamicsWorld->addRigidBody(body);

  return body;
}

btRigidBody *PhysicsWorld::createTerrainBody(btTriangleMesh *terrainMesh)
{
  // Create a static terrain collision shape
  btCollisionShape *terrainShape = new btBvhTriangleMeshShape(terrainMesh, true);

  btTransform terrainTransform;
  terrainTransform.setIdentity();
  terrainTransform.setOrigin(btVector3(0, 0, 0));

  btDefaultMotionState *motionState = new btDefaultMotionState(terrainTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, terrainShape, btVector3(0, 0, 0));

  rbInfo.m_friction = 1.0f;
  rbInfo.m_restitution = 0.0f;

  btRigidBody *body = new btRigidBody(rbInfo);

  dynamicsWorld->addRigidBody(body);

  return body;
}
