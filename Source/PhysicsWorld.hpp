#ifndef TEMPEST_PHYSICS_WORLD_HPP
#define TEMPEST_PHYSICS_WORLD_HPP

#include "btBulletDynamicsCommon.h"

#include "Engine/Scene.h"

#include <memory>

namespace Tempest
{

enum class PhysicsEntityType
{
    DynamicRigid,
    StaticRigid,

    DynamicSoft,
    StaticSoft

};

enum class BasicCollisionGeometry
{
    Box,
    Sphere,
    Capsule
};

class PhysicsWorld
{
public:
    PhysicsWorld();

    void addObject(const InstanceID id, const PhysicsEntityType, const StaticMesh& collisionGeometry, const float4x4& translation);
    void addObject(const InstanceID id, const PhysicsEntityType, const BasicCollisionGeometry collisionGeometry, const float4x4& translation);

private:

    std::unique_ptr<btDefaultCollisionConfiguration> mCollisionConfig;
    std::unique_ptr<btCollisionDispatcher> mCollisionDispatcher;
    std::unique_ptr<btBroadphaseInterface> m_overlapCache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mConstraintSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mWorld;

    btAlignedObjectArray<btCollisionShape*> mCollisionShapes;
};

}

#endif
