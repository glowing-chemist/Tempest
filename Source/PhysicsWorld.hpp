#ifndef TEMPEST_PHYSICS_WORLD_HPP
#define TEMPEST_PHYSICS_WORLD_HPP

#include "btBulletDynamicsCommon.h"

#include "Engine/Scene.h"

#include <memory>
#include <unordered_map>

namespace Tempest
{

enum class PhysicsEntityType
{
    DynamicRigid,
    StaticRigid
};

enum class BasicCollisionGeometry
{
    Box,
    Sphere,
    Capsule,
    Plane
};

class PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void tick(const std::chrono::microseconds diff);

    void addObject(const InstanceID id,
                   const PhysicsEntityType,
                   const StaticMesh& collisionGeometry,
                   const float3& pos,
                   const float3& size,
                   const float mass = 0.0f);

    void addObject(const InstanceID id,
                   const PhysicsEntityType type,
                   const BasicCollisionGeometry collisionGeometry,
                   const float3& pos,
                   const float3& size, const float mass = 0.0f);

    void removeObject(const InstanceID id);

    btRigidBody* getRigidBody(const InstanceID id)
    {
	const uint32_t index = mInstanceMap[id];
	return mRigidBodies[index].get();
    }

    const btAlignedObjectArray<btRigidBody*>& getDynamicObjects() const
    {
        return mWorld->getNonStaticRigidBodies();
    }

    int getManifoldCount() const
    {
        return mCollisionDispatcher->getNumManifolds();
    }

    btPersistentManifold** getManifolds()
    {
        return mCollisionDispatcher->getInternalManifoldPointer();
    }

    struct DefaultShapeCacheEntry
    {
        BasicCollisionGeometry mType;
        float3 mScale;
    };

private:

    btCollisionShape* getCollisionShape(const BasicCollisionGeometry type,
                                        const PhysicsEntityType entitytype,
                                        const float3& scale,
                                        const float mass,
                                        btVector3& outInertia);

    std::unique_ptr<btDefaultCollisionConfiguration> mCollisionConfig;
    std::unique_ptr<btCollisionDispatcher> mCollisionDispatcher;
    std::unique_ptr<btBroadphaseInterface> mOverlapCache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mConstraintSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mWorld;

    std::map<DefaultShapeCacheEntry, uint32_t> mDefaultShapeCache;
    btAlignedObjectArray<btCollisionShape*> mCollisionShapes;

    std::vector<uint32_t> mFreeRigidBodyIndices;
    std::vector<std::unique_ptr<btRigidBody>> mRigidBodies;

    struct InstanceInfo
    {
	InstanceID mID;
	PhysicsEntityType mType;
	uint32_t mIndex;
    };

    std::unordered_map<InstanceID, uint32_t> mInstanceMap;
};

    inline bool operator<(const PhysicsWorld::DefaultShapeCacheEntry& lhs, const PhysicsWorld::DefaultShapeCacheEntry& rhs)
    {
        return (lhs.mScale.x < rhs.mScale.x && lhs.mScale.y < rhs.mScale.y && lhs.mScale.z < rhs.mScale.z) && (static_cast<uint32_t>(lhs.mType) < static_cast<uint32_t>(rhs.mType));
    }

}

#endif
