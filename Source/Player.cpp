#include "Player.hpp"
#include "Controller.hpp"

#include "Engine/Engine.hpp"
#include "Engine/GeomUtils.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

Player::Player(InstanceID id, MeshInstance* inst) :
    mID(id),
    mInstance(inst),
    mPosition(0.0f, 0.0f, 0.0f),
    mDirection(0.0f, 0.0f, 1.0f),
    mCurrentState(Resting)
{
    inst->setInstanceFlags(InstanceFlags::Draw | InstanceFlags::DrawAABB);

    const std::vector<StaticMesh::Bone>& skeleton = inst->mMesh->getSkeleton();
    mHitBoxes.reserve(skeleton.size());
    for(const auto& bone : skeleton)
    {
        HitBox box{};
        box.mOrientatedBoundingBox = bone.mOBB;
        box.mVelocity = float3{0.0f, 0.0f, 0.0f};
        mHitBoxes.push_back(box);
    }

}


void Player::update(const Controller* controller, Engine* eng)
{
    const float x = controller->getLeftAxisX();
    const float z = controller->getLeftAxisY();
    const bool moving = (x != 0.0f || z != 0.0f);

    if(moving)
    {
        mDirection = float3{z, 0.0f,  x};
        mPosition += mDirection;
    }

    float angle = glm::orientedAngle(float2(1.0f, 0.0f), glm::normalize(float2(mDirection.z, mDirection.x)));

    const float4x4 rotation = glm::rotate(angle, float3{0.0f, 1.0f, 0.0f});
    const float4x4 translation = glm::translate(float4x4(1.0f), mPosition);

    mInstance->setTransMatrix(translation * rotation);


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

    if(controller->pressedX())
    {
        if(mCurrentState == Walking)
        {
            eng->terimateAnimation(mID, kWalkingAnimation);
        }

        mCurrentState = Jumping;

        eng->startAnimation(mID, kJumpAnimation);
    }
    else if(controller->releasedX())
    {

    }

    updateHitBoxes(eng);
}


void Player::updateHitBoxes(Engine* eng)
{
    const std::vector<Engine::AnimationEntry> activeAnims = eng->getActiveAnimations();
    for(const auto& anim : activeAnims)
    {
        if(anim.mMesh == mID)
        {
            const std::vector<StaticMesh::Bone>& skeleton = mInstance->mMesh->getSkeleton();

            Animation& animation = mInstance->mMesh->getAnimation(anim.mName);
            std::vector<float4x4> boneTransforms = animation.calculateBoneMatracies(*mInstance->mMesh, anim.mTick);

            for(uint32_t i = 0; i < skeleton.size(); ++i)
            {
                const StaticMesh::Bone& bone = skeleton[i];
                const float4x4& transform = boneTransforms[i];

                float4x4 OBBTransformation = mInstance->getTransMatrix() * transform;
                OBB transformedOBB = bone.mOBB * OBBTransformation;

                HitBox& previousHitBox = mHitBoxes[i];
                previousHitBox.mVelocity = transformedOBB.getCentralPoint() - previousHitBox.mOrientatedBoundingBox.getCentralPoint();
                previousHitBox.mOrientatedBoundingBox = transformedOBB;
            }
        }
    }
}
