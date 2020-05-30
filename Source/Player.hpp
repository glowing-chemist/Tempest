#ifndef PLAYER_HPP
#define PLAYER_HPP

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

private:

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
};


#endif
