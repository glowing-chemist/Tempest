#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Engine/StaticMesh.h"
#include "Engine/Scene.h"


static constexpr const char kWalkingAnimation[] = "Armature|Walk";
static constexpr const char kJumpAnimation[] = "Armature|Jump";

class Controller;


class Player
{
public:

    Player(InstanceID id, MeshInstance* inst);
    ~Player() = default;

    void update(const Controller*, Engine*);

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
        Jumping
    };
    State mCurrentState;

    std::vector<HitBox> mHitBoxes;
};


#endif
