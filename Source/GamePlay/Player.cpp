#include "Player.hpp"
#include "Controller.hpp"
#include "PhysicsWorld.hpp"

#include "Engine/Engine.hpp"
#include "Engine/GeomUtils.h"
#include "Engine/Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace Tempest
{

    Player::Player(InstanceID id, Scene *scene, const float3 &pos, const float3 &dir) :
            mID(id),
            mScene(scene),
            mCamera(nullptr),
            mShadowCamera(nullptr),
            mDirection(dir),
            mCurrentState(Resting),
            mHitBoxes{},
            mCoolDownCounter{0}
    {
        MeshInstance *inst = mScene->getMeshInstance(id);

        inst->setInstanceFlags(InstanceFlags::Draw | InstanceFlags::DrawAABB);
        StaticMesh *mesh = inst->getMesh();

        const AABB &aabb = mesh->getAABB();
        mCentralHeight = aabb.getCentralPoint().y * inst->getScale().y;

        const std::vector<Bone> &skeleton = mesh->getSkeleton();
        mHitBoxes.reserve(skeleton.size());
        for (const auto &bone : skeleton)
        {
            HitBox box{};
            box.mOrientatedBoundingBox = bone.mOBB;
            box.mVelocity = float3{0.0f, 0.0f, 0.0f};
            mHitBoxes.push_back(box);
        }
    }


    void Player::update(const Controller *controller, RenderEngine *eng, Tempest::PhysicsWorld *world) {
        PROFILER_EVENT();
        /*
        if (moving && sprinting && mCurrentState == Resting) {
            eng->startAnimation(mID, kSprintAnimation, true, 2.0f);
            mCurrentState = Sprinting;
        } else if (moving && sprinting && mCurrentState == Walking) {
            eng->terimateAnimation(mID, kWalkingAnimation);
            eng->startAnimation(mID, kSprintAnimation, true, 2.0f);
            mCurrentState = Sprinting;
        } else if ((mCurrentState == Resting || mCurrentState == Sprinting) && moving && !sprinting) {
            if (mCurrentState == Sprinting)
                eng->terimateAnimation(mID, kSprintAnimation);
            eng->startAnimation(mID, kWalkingAnimation, true);
            mCurrentState = Walking;
        } else if ((mCurrentState == Walking || mCurrentState == Sprinting) && !moving) {
            eng->terimateAnimation(mID, kWalkingAnimation);
            eng->terimateAnimation(mID, kSprintAnimation);
            mCurrentState = Resting;
        }

        if (controller->pressedX() && mCurrentState != Jumping) {
            if (mCurrentState == Walking || mCurrentState == Sprinting) {
                eng->terimateAnimation(mID, kWalkingAnimation);
                eng->terimateAnimation(mID, kSprintAnimation);
            }

            mCurrentState = Jumping;

            eng->startAnimation(mID, kJumpAnimation, false);
            if (!body->isActive())
                body->activate(true);
            body->applyCentralImpulse({0.0f, 400.0f, 0.0f});

            mCoolDownCounter = 80;
        } else if (controller->releasedX()) {

        }

        updateHitBoxes(eng);

        if (!body->isActive())
            body->activate(true);
            */
    }

    void Player::updateCameras(Controller* controller)
    {
        float3 instancePosition = mScene->getInstancePosition(mID);

        // update attached camera
        if (mCamera)
        {
            const float cx = controller->getRightAxisX();
            const float cz = controller->getRightAxisY();

            float3 position = mCamera->getPosition();
            float3 direction = mCamera->getDirection();
            const float3 right = mCamera->getRight();

            glm::float4x4 rotation = glm::rotate(-0.1f * cx, float3(0.0f, 1.0f, 0.0f)) * glm::rotate(-0.1f * cz, right);
            const float4 positionOffset = float4(
                    float3((mArmatureLength * direction) + float3(0.0f, -mCentralHeight * 2.0f, 0.0f)), 0.0f);
            position = instancePosition - float3(positionOffset * rotation);
            direction = glm::normalize(float4(direction, 0.0f) * rotation);

            mCamera->setPosition(position);
            mCamera->setDirection(direction);
        }

        if (mShadowCamera)
        {
            mShadowCamera->setPosition({instancePosition.x, 10.0f, instancePosition.z});
        }
    }


    void Player::updateHitBoxes(RenderEngine *eng)
    {
        MeshInstance *instance = mScene->getMeshInstance(mID);
        BELL_ASSERT(instance, "invalid mesh ID")

        bool foundAnim = instance->getActiveAnimation();

        const std::vector<Bone> &skeleton = instance->getMesh()->getSkeleton();

        std::vector<float4x4> boneTransforms = instance->tickAnimation(eng->getDevice()->getCurrentSubmissionIndex() / 60.0);

        for (uint32_t i = 0; i < skeleton.size(); ++i)
        {
            const Bone &bone = skeleton[i];
            const float4x4 &transform = boneTransforms[i];

            const float4x4 OBBTransformation = instance->getTransMatrix() * transform;
            const OBB transformedOBB = bone.mOBB * OBBTransformation;

            HitBox &previousHitBox = mHitBoxes[i];
            previousHitBox.mVelocity =
                        transformedOBB.getCentralPoint() - previousHitBox.mOrientatedBoundingBox.getCentralPoint();
                        previousHitBox.mOrientatedBoundingBox = transformedOBB;
        }

        if (!foundAnim)
        {
            for (uint32_t i = 0; i < skeleton.size(); ++i)
            {
                const Bone &bone = skeleton[i];

                const float4x4 OBBTransformation = instance->getTransMatrix();
                const OBB transformedOBB = bone.mOBB * OBBTransformation;

                HitBox &previousHitBox = mHitBoxes[i];
                previousHitBox.mVelocity =
                        transformedOBB.getCentralPoint() - previousHitBox.mOrientatedBoundingBox.getCentralPoint();
                previousHitBox.mOrientatedBoundingBox = transformedOBB;
            }
        }
    }
}
