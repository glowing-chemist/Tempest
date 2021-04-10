#include "PhysicsWorld.hpp"
#include "DebugRenderer.hpp"
#include "Engine/Engine.hpp"

#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"

#include "glm/gtc/type_ptr.hpp"

#include "Core/Profiling.hpp"

namespace Tempest
{

PhysicsWorld::PhysicsWorld(RenderEngine* debugDraw) :
    mDebugRenderer(debugDraw)
{
    mCollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();

    mCollisionDispatcher = std::make_unique<btCollisionDispatcher>(mCollisionConfig.get());

    mOverlapCache = std::make_unique<btDbvtBroadphase>();

    mConstraintSolver = std::make_unique<btSequentialImpulseConstraintSolver>();

    mWorld = std::make_unique<btDiscreteDynamicsWorld>(mCollisionDispatcher.get(), mOverlapCache.get(), mConstraintSolver.get(), mCollisionConfig.get());

    mWorld->setGravity(btVector3(0.0f, -9.8f, 0.0f));
    mWorld->setWorldUserInfo(this);
    if(debugDraw)
        mWorld->setDebugDrawer(&mDebugRenderer);
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
    PROFILER_EVENT();

    mWorld->stepSimulation(float(diff.count()) / 1000000.0f, 10);
}

void PhysicsWorld::updateDynamicObjects(Scene* scene)
{
    const btAlignedObjectArray<btRigidBody*>& rigidBodies = mWorld->getNonStaticRigidBodies();

    for(uint32_t i = 0; i < rigidBodies.size(); ++i)
    {
        const btRigidBody* rigidBody = rigidBodies[i];
        const InstanceID id = rigidBody->getUserIndex();
        MeshInstance* instance = scene->getMeshInstance(id);

        const btTransform& transform = rigidBody->getWorldTransform();
        const btVector3& position = transform.getOrigin();
        const btQuaternion& rotation = transform.getRotation();

        instance->setPosition({position.x(), position.y(), position.z()});
        instance->setRotation({rotation.w(), rotation.x(), rotation.y(), rotation.z()});
    }
}

void PhysicsWorld::addObject(const InstanceID id,
                             const PhysicsEntityType type,
                             const BasicCollisionGeometry collisionGeometry,
                             const float3& pos,
                             const quat& rot,
                             const float3& size,
                             const float mass,
                             const float restitution)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    if(collisionGeometry != BasicCollisionGeometry::Plane)
        transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

    btVector3 localInertia;
    btCollisionShape* shape = getCollisionShape(collisionGeometry, type, size, mass, localInertia);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
    rbInfo.m_restitution = restitution;
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setUserIndex(id);

    if(type == PhysicsEntityType::Kinematic)
    {
        body->setCollisionFlags( body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        body->setActivationState( DISABLE_DEACTIVATION );
    }

    if(collisionGeometry == BasicCollisionGeometry::Capsule)
        body->setAngularFactor({0.0f, 1.0f, 0.0f});

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

void PhysicsWorld::addObject(const InstanceID id,
                             const PhysicsEntityType type,
                             const StaticMesh* collisionGeometry,
                             const float3& pos,
                             const quat& rot,
                             const float3& scale)
{
    btRigidBody* body = nullptr;
    btCollisionShape* shape;

    auto it = mConvexMeshCache.find(collisionGeometry);
    if(it != mConvexMeshCache.end())
    {
        const uint32_t shapeIndex = it->second;
        shape = mCollisionShapes[shapeIndex];
    }
    else
    {
        const uint32_t stride = collisionGeometry->getVertexStride();
        const std::vector<SubMesh>& subMeshes = collisionGeometry->getSubMeshes();
        btCompoundShape* compoundShape = new btCompoundShape(true, subMeshes.size());
        for(const auto& subMesh : subMeshes)
        {
            btConvexHullShape* hullShape = new btConvexHullShape();
            const unsigned char *vertexData = collisionGeometry->getVertexData().data() + (subMesh.mVertexOffset * stride);
            for (uint32_t i = 0; i < subMesh.mVertexCount; ++i)
            {
                const float4* position = reinterpret_cast<const float4 *>(vertexData);
                const float4 scaledPosition = (subMesh.mTransform * *position) * float4(scale, 1.0f);
                hullShape->addPoint(btVector3(scaledPosition.x, scaledPosition.y, scaledPosition.z), false);

                vertexData += stride;
            }
            hullShape->recalcLocalAabb();

            mCollisionShapes.push_back(hullShape);

            btTransform subTransform{};
            subTransform.setIdentity();
            compoundShape->addChildShape(subTransform, hullShape);
        }
        compoundShape->recalculateLocalAabb();
        mConvexMeshCache.insert({collisionGeometry, mCollisionShapes.size()});
        mCollisionShapes.push_back(compoundShape);

        shape = compoundShape;
    }

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

    btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
    btDefaultMotionState *myMotionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, shape, localInertia);
    body = new btRigidBody(rbInfo);
    body->setUserIndex(id);

    if (mFreeRigidBodyIndices.empty())
    {
        const uint32_t index = mRigidBodies.size();
        mRigidBodies.emplace_back(body);
        mInstanceMap[id] = index;
    } else
    {
        const uint32_t index = mFreeRigidBodyIndices.back();
        mFreeRigidBodyIndices.pop_back();

        mRigidBodies[index] = std::unique_ptr<btRigidBody>(body);
    }

    mWorld->addRigidBody(body);
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

btCollisionShape* PhysicsWorld::getCollisionShape(const BasicCollisionGeometry type, const PhysicsEntityType entitytype, const float3& scale, const float mass, btVector3& outInertia)
{
    btCollisionShape* shape;
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
                const float3 halfExtent = scale / 2.0f;
                shape = new btBoxShape(btVector3(halfExtent.x, halfExtent.y, halfExtent.z));
                break;
            }

            case BasicCollisionGeometry::Sphere:
            {
                shape = new btSphereShape(scale.x / 2.0f);
                break;
            }

            case BasicCollisionGeometry::Capsule:
            {
                shape = new btCapsuleShape(scale.x / 2.0f, scale.y);
                break;
            }

            case BasicCollisionGeometry::Plane:
            {
                shape = new btStaticPlaneShape({0.0f, 1.0f, 0.0f}, 0.0f);
                break;
            }
        }

        mDefaultShapeCache[{type, scale}] =  mCollisionShapes.size();
        mCollisionShapes.push_back(shape);
    }

    outInertia = btVector3(0.0f, 0.0f, 0.0f);
    if (entitytype == PhysicsEntityType::DynamicRigid)
        shape->calculateLocalInertia(btScalar(mass), outInertia);

    return shape;
}

    void PhysicsWorld::drawDebugAABB(RenderEngine* eng)
    {
        for(auto [id, index] : mInstanceMap)
        {
            const std::unique_ptr<btRigidBody>& body = mRigidBodies[index];
            btVector3 min, max;
            body->getAabb(min, max);

            const AABB bounds({min.x(), min.y(), min.z(), 1.0f},
                        {max.x(), max.y(), max.z(), 1.0f});

            eng->addDebugAABB(bounds);
        }
    }


    void PhysicsWorld::drawDebugObjects()
    {
        mWorld->debugDrawWorld();
    }


    void PhysicsWorld::drawDebugObject(const InstanceID id)
    {
        if(auto it = mInstanceMap.find(id); it != mInstanceMap.end())
        {
            const uint32_t rigidIndex = it->second;
            const std::unique_ptr<btRigidBody> &body = mRigidBodies[rigidIndex];
            const btCollisionShape *shape = body->getCollisionShape();

            mWorld->debugDrawObject(body->getWorldTransform(), shape, {1.f, 0.0f, 0.0f});
        }
    }


    void PhysicsWorld::setInstancePosition(const InstanceID id, const float3& v)
    {
        btRigidBody* body = getRigidBody(id);
        if(body)
        {
            btMotionState* state = body->getMotionState();
            btTransform &transform = body->getWorldTransform();
            transform.setOrigin({v.x, v.y, v.z});
            state->setWorldTransform(transform);

            if (!body->isActive())
                body->activate(true);
        }
    }


    void PhysicsWorld::translateInstance(const InstanceID id, const float3& v)
    {
        btRigidBody* body = getRigidBody(id);
        if(body) {
            btMotionState* state = body->getMotionState();
            btTransform &transform = body->getWorldTransform();
            btVector3& origin = transform.getOrigin();
            transform.setOrigin({origin.x() + v.x, origin.y() + v.y, origin.z() + v.z});
            state->setWorldTransform(transform);

            if (!body->isActive())
                body->activate(true);
        }
    }

    void PhysicsWorld::setInstanceLinearVelocity(const InstanceID id, const float3& v)
    {
        btRigidBody* body = getRigidBody(id);
        if(body) {
            body->setLinearVelocity({v.x, v.y, v.z});

            if (!body->isActive())
                body->activate(true);
        }
    }

    void PhysicsWorld::setInstanceRotation(const InstanceID id, const quat& rot)
    {
        btRigidBody* body = getRigidBody(id);
        if(body) {
            btMotionState* state = body->getMotionState();
            btTransform &transform = body->getWorldTransform();
            btVector3& origin = transform.getOrigin();
            transform.setRotation({rot.x, rot.y, rot.z, rot.w});
            state->setWorldTransform(transform);

            if (!body->isActive())
                body->activate(true);
        }
    }


}
