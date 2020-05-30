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
}
