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

        void drawInstanceWindow(Level*, const InstanceID);

    private:

    std::filesystem::path mWorkingDir;
    std::vector<std::string> mScriptNames;

    struct InstanceEntry
    {
        InstanceEntry() :
            mHasScript(false),
            mScriptIndex{0},
            mGuizmoIndex{0},
            mHasCollider{false},
            mCollisionGeom{BasicCollisionGeometry::Box} {}

        bool mHasScript;
        uint32_t mScriptIndex;
        int mGuizmoIndex;
        bool mHasCollider;
        BasicCollisionGeometry mCollisionGeom;

    };
    std::unordered_map<InstanceID, InstanceEntry> mInstanceInfo;

    };
}

#endif