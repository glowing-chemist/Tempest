#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Engine/StaticMesh.h"
#include "Engine/Scene.h"

class  Scene;

static constexpr const char kWalkingAnimation[] = "Armature|Armature|Armature|Walk|Armature|Walk";
static constexpr const char kSprintAnimation[] = "Armature|Armature|mixamo.com|Layer0";
static constexpr const char kJumpAnimation[] = "Armature|Armature|mixamo.com|Layer1";


namespace Tempest {
    class PhysicsWorld;
    class Controller;


    class Player {
    public:

        Player(InstanceID id, Scene *scene, const float3 &pos, const float3 &dir);

        ~Player() = default;

        void update(const Controller *, RenderEngine *, Tempest::PhysicsWorld *world);
        void updateCameras(Controller*);

        struct HitBox {
            HitBox() :
                    mOrientatedBoundingBox(),
                    mVelocity(0.0f, 0.0f, 0.0f) {}

            OBB mOrientatedBoundingBox;
            float3 mVelocity;
        };

        const std::vector<HitBox> &getHitBoxes() const {
            return mHitBoxes;
        }

        const float3 &getDirection() const {
            return mDirection;
        }

        void attachCamera(Camera &cam, const float armatureLength) {
            mArmatureLength = armatureLength;
            mCamera = &cam;
        }

        void detatchCamera() {
            mCamera = nullptr;
        }

        void attachShadowCamera(Camera &cam) {
            mShadowCamera = &cam;
        }

        void detatchShadowCamera() {
            mShadowCamera = nullptr;
        }

    private:

        void updateHitBoxes(RenderEngine *eng);

        InstanceID mID;
        Scene *mScene;

        float mCentralHeight;
        float mArmatureLength;
        Camera *mCamera;
        Camera *mShadowCamera;

        float3 mDirection;

        enum State {
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

}
#endif
