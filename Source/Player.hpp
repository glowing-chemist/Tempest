#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Engine/StaticMesh.h"
#include "Engine/Scene.h"


static constexpr const char kWalkingAnimation[] = "Armature|Walk";
static constexpr const char kJumpAnimation[] = "Armature|Kick";

class Controller;


class Player
{
public:

    Player(InstanceID id, MeshInstance* inst);
    ~Player() = default;

    void update(const Controller*, Engine*);

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

private:

    void updateHitBoxes(Engine *eng);

    InstanceID mID;
    MeshInstance* mInstance;

    float3 mPosition;
    float3 mDirection;

    enum State
    {
        Resting,
        Walking,
        Jumping,
        Kicking
    };
    State mCurrentState;

    std::vector<HitBox> mHitBoxes;

    size_t mCoolDownCounter;
};


#endif
