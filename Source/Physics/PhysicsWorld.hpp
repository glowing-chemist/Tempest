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
    StaticRigid,
    Kinematic
};

enum class CollisionMeshType
{
    Concave,
    Convex
};

enum class BasicCollisionGeometry
{
    Box = 0,
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
    void updateDynamicObjects(Scene*);

    void addObject(const InstanceID id,
                   const CollisionMeshType,
                   const StaticMesh& collisionGeometry,
                   const float3& pos,
                   const quat& rot,
                   const float3& scale);

    void addObject(const InstanceID id,
                   const PhysicsEntityType type,
                   const BasicCollisionGeometry collisionGeometry,
                   const float3& pos,
                   const quat& rot,
                   const float3& size,
                   const float mass = 0.0f,
                   const float restitution = 0.0f);

    void removeObject(const InstanceID id);

    btRigidBody* getRigidBody(const InstanceID id)
    {
        if(auto it = mInstanceMap.find(id); it != mInstanceMap.end())
        {
            const uint32_t index = it->second;
            return mRigidBodies[index].get();
        }
        else
            return nullptr;
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

    void setInstancePosition(const InstanceID, const float3&);
    void translateInstance(const InstanceID, const float3&);
    void setInstanceLinearVelocity(const InstanceID, const float3&);

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
    btAlignedObjectArray<btStridingMeshInterface*> mCollisionMeshes;

    std::vector<uint32_t> mFreeRigidBodyIndices;
    std::vector<std::unique_ptr<btRigidBody>> mRigidBodies;

    std::unordered_map<InstanceID, uint32_t> mInstanceMap;
};

    inline bool operator<(const PhysicsWorld::DefaultShapeCacheEntry& lhs, const PhysicsWorld::DefaultShapeCacheEntry& rhs)
    {
        auto hash = [](const PhysicsWorld::DefaultShapeCacheEntry& e)
        {
            std::hash<float> hasher{};
            uint64_t h = hasher(e.mScale.x);
            h ^= hasher(e.mScale.y);
            h ^= hasher(e.mScale.z);
            h ^= std::hash<uint32_t>{}(static_cast<uint32_t>(e.mType));

            return h;
        };

        return hash(lhs) < hash(rhs);
    }

}

#endif
