#include "PhysicsWorld.hpp"

#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btConcaveShape.h"
#include "BulletCollision/CollisionShapes/btConvexShape.h"
#include "BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h"

#include "glm/gtc/type_ptr.hpp"

#include "Core/Profiling.hpp"

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
                             const CollisionMeshType type,
                             const StaticMesh &collisionGeometry,
                             const float3& pos,
                             const quat& rot,
                             const float3& scale)
{
    std::vector<float3> verticies(collisionGeometry.getVertexCount());
    std::vector<int> indicies{};
    indicies.reserve(collisionGeometry.getIndexData().size());
    std::transform(collisionGeometry.getIndexData().begin(), collisionGeometry.getIndexData().end(), std::back_inserter(indicies),
    [](const uint32_t i) {return static_cast<int>(i);} );
    const uint32_t stride = collisionGeometry.getVertexStride();
    const unsigned char* vertexData = collisionGeometry.getVertexData().data();
    for(uint32_t i = 0; i < verticies.size(); ++i)
    {
        const float3* position = reinterpret_cast<const float3*>(vertexData);
        verticies[i] = *position * scale;
        vertexData += stride;
    }

    btTriangleIndexVertexArray* vertexArray = new btTriangleIndexVertexArray(indicies.size() / 3,
                                                                            indicies.data(), sizeof(int),
                                                                            verticies.size(), &verticies.data()->x, sizeof(float3));
    mCollisionMeshes.push_back(vertexArray);

    btCollisionShape* shape = nullptr;
    if(type == CollisionMeshType::Concave)
    {
        BELL_TRAP;
    }
    else
    {
        shape = new btConvexTriangleMeshShape(vertexArray);
    }

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

    btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
    btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, shape, localInertia);
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
                float3 halfExtent = scale / 2.0f;
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

        mDefaultShapeCache[{type, scale}] = mCollisionShapes.size();
        mCollisionShapes.push_back(shape);
    }

    outInertia = btVector3(0.0f, 0.0f, 0.0f);
    if (entitytype == PhysicsEntityType::DynamicRigid)
        shape->calculateLocalInertia(btScalar(mass), outInertia);

    return shape;
}

    void PhysicsWorld::setInstancePosition(const InstanceID id, const float3& v)
    {
        btRigidBody* body = getRigidBody(id);
        if(body)
        {
            btTransform &transform = body->getWorldTransform();
            transform.setOrigin({v.x, v.y, v.z});
            body->setWorldTransform(transform);

            if (!body->isActive())
                body->activate(true);
        }
    }


    void PhysicsWorld::translateInstance(const InstanceID id, const float3& v)
    {
        btRigidBody* body = getRigidBody(id);
        if(body) {
            btTransform &transform = body->getWorldTransform();
            btVector3 origin = transform.getOrigin();
            transform.setOrigin({origin.x() + v.x, origin.y() + v.y, origin.z() + v.z});
            body->setWorldTransform(transform);

            if (!body->isActive())
                body->activate(true);
        }
    }

}
