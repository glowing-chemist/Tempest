#include "Player.hpp"
#include "Controller.hpp"
#include "PhysicsWorld.hpp"

#include "Engine/Engine.hpp"
#include "Engine/GeomUtils.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

Player::Player(InstanceID id, MeshInstance* inst, const float3 &pos, const float3 &dir) :
    mID(id),
    mInstance(inst),
    mCamera(nullptr),
    mPosition(pos),
    mDirection(dir),
    mCurrentState(Resting),
    mHitBoxes{},
    mCoolDownCounter{0}
{
    inst->setInstanceFlags(InstanceFlags::Draw | InstanceFlags::DrawAABB);
    StaticMesh* mesh = inst->getMesh();

    const AABB& aabb = mesh->getAABB();
    mCentralHeight = aabb.getCentralPoint().y;

    const std::vector<StaticMesh::Bone>& skeleton = mesh->getSkeleton();
    mHitBoxes.reserve(skeleton.size());
    for(const auto& bone : skeleton)
    {
        HitBox box{};
        box.mOrientatedBoundingBox = bone.mOBB;
        box.mVelocity = float3{0.0f, 0.0f, 0.0f};
        mHitBoxes.push_back(box);
    }

}


void Player::update(const Controller* controller, Engine* eng, Tempest::PhysicsWorld* world)
{
    PROFILER_EVENT();

    const float x = controller->getLeftAxisX() * 0.01f;
    const float z = controller->getLeftAxisY() * 0.01f;
    const bool moving = (x != 0.0f || z != 0.0f) && mCoolDownCounter == 0;

    // update attached camera
    if(mCamera)
    {
        const float cx = controller->getRighAxisX();
        const float cz = controller->getRighAxisY();

        float3 position = mCamera->getPosition();
        float3 direction = mCamera->getDirection();
        const float3 right = mCamera->getRight();

        glm::float4x4 rotation = glm::rotate(-0.1f * cx, float3(0.0f, 1.0f, 0.0f)) * glm::rotate(-0.1f * cz, right);
        const float4 positionOffset = float4(float3((mArmatureLength * direction) + float3(0.0f, -mCentralHeight - mPosition.y, 0.0f)), 0.0f);
        position = mPosition - float3(positionOffset * rotation);
        direction = glm::normalize(float4(direction, 0.0f) * rotation);

        mCamera->setPosition(position);
        mCamera->setDirection(direction);
    }

    if(mShadowCamera && moving)
    {
        float3 position = mShadowCamera->getPosition();
        position += float3(mDirection.x, 0.0f, mDirection.z);

        mShadowCamera->setPosition(position);
    }

    if(mCoolDownCounter > 0)
        --mCoolDownCounter;

    if(mCoolDownCounter == 0 && mCurrentState == Jumping)
        mCurrentState = Resting;

    if(moving)
    {
        if (mCamera)
        {
            float3 direction = mCamera->getDirection();
            direction.y = 0.0f;
            direction = glm::normalize(direction);
            const float3 right = mCamera->getRight();
            mDirection = (-z * direction) + (x * right);
        } else
            mDirection = float3{z, 0.0f, x};
        mPosition += mDirection;

        btRigidBody* body = world->getRigidBody(mID);
        btTransform& transform = body->getWorldTransform();
        transform.setOrigin({mPosition.x, mCentralHeight + mPosition.y,mPosition.z});
        body->setWorldTransform(transform);
        body->activate(true);

        updateRenderinstance();
    }

    if(mCurrentState == Resting && moving)
    {
        eng->startAnimation(mID, kWalkingAnimation, true);
        mCurrentState = Walking;
    }
    else if(mCurrentState == Walking && !moving)
    {
        eng->terimateAnimation(mID, kWalkingAnimation);
        mCurrentState = Resting;
    }

    if(controller->pressedX() && mCurrentState != Jumping)
    {
        if(mCurrentState == Walking)
        {
            eng->terimateAnimation(mID, kWalkingAnimation);
        }

        mCurrentState = Jumping;

        eng->startAnimation(mID, kJumpAnimation, false, 4.0f);

        mCoolDownCounter = 40;
    }
    else if(controller->releasedX())
    {

    }

    updateHitBoxes(eng);
}


void Player::undoMove()
{
    if(mCamera)
    {
        float3 position = mCamera->getPosition();
        position = position - mDirection * 2.0f;

        mCamera->setPosition(position);
    }

    if(mShadowCamera)
    {
        float3 position = mShadowCamera->getPosition();
        position = position - mDirection * 2.0f;

        mShadowCamera->setPosition(position);
    }

    mPosition -= mDirection * 4.0f;
}


void Player::updateHitBoxes(Engine* eng)
{
    const std::vector<Engine::SkeletalAnimationEntry>& activeAnims = eng->getActiveSkeletalAnimations();
    bool foundAnim = false;
    const std::vector<StaticMesh::Bone>& skeleton = mInstance->getMesh()->getSkeleton();
    for(const auto& anim : activeAnims)
    {
        if(anim.mMesh == mID)
        {
            foundAnim = true;

            SkeletalAnimation& animation = mInstance->getMesh()->getSkeletalAnimation(anim.mName);
            std::vector<float4x4> boneTransforms = animation.calculateBoneMatracies(*mInstance->getMesh(), anim.mTick);

            for(uint32_t i = 0; i < skeleton.size(); ++i)
            {
                const StaticMesh::Bone& bone = skeleton[i];
                const float4x4& transform = boneTransforms[i];

                const float4x4 OBBTransformation = mInstance->getTransMatrix() * transform;
                const OBB transformedOBB = bone.mOBB * OBBTransformation;

                HitBox& previousHitBox = mHitBoxes[i];
                previousHitBox.mVelocity = transformedOBB.getCentralPoint() - previousHitBox.mOrientatedBoundingBox.getCentralPoint();
                previousHitBox.mOrientatedBoundingBox = transformedOBB;
            }
        }
    }

    if(!foundAnim)
    {
        for(uint32_t i = 0; i < skeleton.size(); ++i)
        {
            const StaticMesh::Bone& bone = skeleton[i];

            const float4x4 OBBTransformation = mInstance->getTransMatrix();
            const OBB transformedOBB = bone.mOBB * OBBTransformation;

            HitBox& previousHitBox = mHitBoxes[i];
            previousHitBox.mVelocity = transformedOBB.getCentralPoint() - previousHitBox.mOrientatedBoundingBox.getCentralPoint();
            previousHitBox.mOrientatedBoundingBox = transformedOBB;
        }
    }
}


void Player::updateRenderinstance()
{
    float angle = glm::orientedAngle(float2(1.0f, 0.0f), glm::normalize(float2(mDirection.z, mDirection.x)));

    const float4x4 rotation = glm::rotate(angle, float3{0.0f, 1.0f, 0.0f});
    const float4x4 translation = glm::translate(float4x4(1.0f), mPosition);

    mInstance->setTransMatrix(translation * rotation);
}
