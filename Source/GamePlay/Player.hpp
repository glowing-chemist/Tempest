#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Engine/StaticMesh.h"
#include "Engine/Scene.h"


static constexpr const char kWalkingAnimation[] = "Armature|Armature|Armature|Walk|Armature|Walk";
static constexpr const char kSprintAnimation[] = "Armature|Armature|mixamo.com|Layer0";
static constexpr const char kJumpAnimation[] = "Armature|Armature|mixamo.com|Layer1";

class Controller;

namespace Tempest
{
    class PhysicsWorld;
}

class Player
{
public:

    Player(InstanceID id, MeshInstance* inst, const float3& pos, const float3& dir);
    ~Player() = default;

    void update(const Controller*, RenderEngine*, Tempest::PhysicsWorld* world);

    void applyForce(const float3& dir)
    {
        mPosition += dir;
    }

    struct HitBox
    {
        HitBox() :
            mOrientatedBoundingBox(),
            mVelocity(0.0f, 0.0f, 0.0f)
        {}

        OBB mOrientatedBoundingBox;
        float3 mVelocity;
    };

    const std::vector<HitBox>& getHitBoxes() const
    {
        return mHitBoxes;
    }

    const float3& getDirection() const
    {
        return mDirection;
    }

    const float3& getPosition() const
    {
        return mPosition;
    }

    void attachCamera(Camera& cam, const float armatureLength)
    {
        mArmatureLength = armatureLength;
        mCamera = &cam;
    }

    void detatchCamera()
    {
        mCamera = nullptr;
    }

    void attachShadowCamera(Camera& cam)
    {
        mShadowCamera = &cam;
    }

    void detatchShadowCamera()
    {
        mShadowCamera = nullptr;
    }

    void undoMove();

private:

    void updateHitBoxes(RenderEngine *eng);
    void updateRenderinstance();

    InstanceID mID;
    MeshInstance* mInstance;

    float mCentralHeight;
    float mArmatureLength;
    Camera* mCamera;
    Camera* mShadowCamera;

    float3 mPosition;
    float3 mDirection;

    enum State
    {
        Resting,
        Walking,
        Sprinting,
        Jumping,
        Kicking
    };
    State mCurrentState;

    std::vector<HitBox> mHitBoxes;

    size_t mCoolDownCounter;
};


#endif