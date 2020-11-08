#include "PhysicsWorld.hpp"

#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"

#include "glm/gtc/type_ptr.hpp"


namespace Tempest
{

PhysicsWorld::PhysicsWorld()
{
    mCollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();

    mCollisionDispatcher = std::make_unique<btCollisionDispatcher>(mCollisionConfig.get());

    mOverlapCache = std::make_unique<btDbvtBroadphase>();

    mConstraintSolver = std::make_unique<btSequentialImpulseConstraintSolver>();

    mWorld = std::make_unique<btDiscreteDynamicsWorld>(mCollisionDispatcher.get(), mOverlapCache.get(), mConstraintSolver.get(), mCollisionConfig.get());

    mWorld->setGravity(btVector3(0.0f, -9.8f, 0.0f));
    mWorld->setWorldUserInfo(this);
}


PhysicsWorld::~PhysicsWorld()
{
    for(int i = 0; i < mCollisionShapes.size(); ++i)
    {
        btCollisionShape* shape = mCollisionShapes[i];
        delete shape;
    }
}

void PhysicsWorld::tick(const std::chrono::microseconds diff)
{
    mWorld->stepSimulation(float(diff.count()) / 1000000.0f, 10);
}

void PhysicsWorld::addObject(const InstanceID id, const PhysicsEntityType type, const BasicCollisionGeometry collisionGeometry, const float4x4& transformation, const float mass)
{
    btTransform transform;
    transform.setIdentity();
    const float4 origin = transformation[3];
    transform.setOrigin(btVector3(origin.x, origin.y, origin.z));

    btVector3 localInertia;
    btCollisionShape* shape = getCollisionShape(collisionGeometry, type, transformation, mass, localInertia);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setUserIndex(id);

    if(mFreeRigidBodyIndices.empty())
    {
        const uint32_t index = mRigidBodies.size();
        mRigidBodies.emplace_back(body);
        mInstanceMap[id] = index;
    }
    else
    {
        const uint32_t index = mFreeRigidBodyIndices.back();
        mFreeRigidBodyIndices.pop_back();

        mRigidBodies[index] = std::unique_ptr<btRigidBody>(body);
    }

    mWorld->addRigidBody(body);
}

void PhysicsWorld::addObject(const InstanceID id, const PhysicsEntityType, const StaticMesh &collisionGeometry, const float4x4 &translation, const float mass)
{
    BELL_TRAP;
}

void PhysicsWorld::removeObject(const InstanceID id)
{
    const uint32_t index = mInstanceMap[id];
    btRigidBody* body = mRigidBodies[index].get();
    mInstanceMap.erase(id);
    mWorld->removeRigidBody(body);
    delete body;
    mRigidBodies[index] = nullptr;
    mFreeRigidBodyIndices.push_back(index);
}

btCollisionShape* PhysicsWorld::getCollisionShape(const BasicCollisionGeometry type, const PhysicsEntityType entitytype, const float4x4& transformation, const float mass, btVector3& outInertia)
{
    btCollisionShape* shape;
    float3 scale = float3(transformation[0].x, transformation[1].y, transformation[2].z);
    auto it = mDefaultShapeCache.find({type, scale});
    if(it != mDefaultShapeCache.end())
    {
        shape = mCollisionShapes[it->second];
    }
    else
    {
        switch(type)
        {
            case BasicCollisionGeometry::Box:
            {
                float3 halfExtent = scale / 2.0f;
                shape = new btBoxShape(btVector3(halfExtent.x, halfExtent.y, halfExtent.z));
                break;
            }

            case BasicCollisionGeometry::Sphere:
            {
                shape = new btSphereShape(scale.x);
                break;
            }

            case BasicCollisionGeometry::Capsule:
            {
                shape = new btCapsuleShape(scale.x, scale.y);
                break;
            }
        }

        mDefaultShapeCache[{type, scale}] = mCollisionShapes.size();
        mCollisionShapes.push_back(shape);
    }

    outInertia = btVector3(0.0f, 0.0f, 0.0f);
    if (entitytype == PhysicsEntityType::DynamicRigid)
        shape->calculateLocalInertia(btScalar(mass), outInertia);

    return shape;
}

}
