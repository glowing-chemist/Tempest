#ifndef INSTANCE_WINDOW_HPP
#define INSTANCE_WINDOW_HPP

#include "Engine/Scene.h"
#include "PhysicsWorld.hpp"

#include <unordered_map>

namespace Tempest
{
    class Level;

    class InstanceWindow
    {
    public:
        InstanceWindow(const std::filesystem::path& dir);

        bool drawInstanceWindow(Level*, const InstanceID);

        void setInstanceScript(const InstanceID, const std::string& script);
        void setInstanceCollider(const InstanceID, const BasicCollisionGeometry, const float mass, const bool dynamic, const float restitution);

        struct InstanceEntry
        {
            InstanceEntry() :
                    mHasScript(false),
                    mScriptIndex{0},
                    mGuizmoIndex{0},
                    mHasCollider{false},
                    mCollisionGeom{BasicCollisionGeometry::Box},
                    mMass{0.f},
                    mDynamic{false},
                    mRestitution{0.0f} {}

            bool mHasScript;
            uint32_t mScriptIndex;
            int mGuizmoIndex;
            bool mHasCollider;
            BasicCollisionGeometry mCollisionGeom;
            float mMass;
            bool mDynamic;
            float mRestitution;

        };

        const std::unordered_map<InstanceID, InstanceEntry>& getInstanceInfo() const
        {
            return mInstanceInfo;
        }

        const std::vector<std::string>& getScriptNames() const
        {
            return mScriptNames;
        }

    private:

    std::filesystem::path mWorkingDir;
    std::vector<std::string> mScriptNames;

    std::unordered_map<InstanceID, InstanceEntry> mInstanceInfo;

    };
}

#endif