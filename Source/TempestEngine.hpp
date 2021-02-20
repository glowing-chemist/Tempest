#ifndef TEMPEST_ENGINE_HPP
#define TEMPEST_ENGINE_HPP

#include <filesystem>
#include "Engine/GeomUtils.h"
#include "Engine/Scene.h"

class RenderEngine;
class Scene;
struct GLFWwindow;

namespace Tempest
{
    class ScriptEngine;
    class RenderThread;
    class PhysicsWorld;
    class Level;

class TempestEngine
{
public:
    TempestEngine(GLFWwindow* window, const std::filesystem::path& rootDir);
    ~TempestEngine();

    // Load level
    void LoadLevel(const std::string& path);

    // main loop to be called once c++ side.
    void run();

    // lua scripting hooks.
    void translateInstance(const InstanceID, const float3&);
    float3 getInstancePosition(const InstanceID) const;
    void   setInstancePosition(const InstanceID, const float3&);

    void startAnimation(const InstanceID id, const std::string& name, const bool loop, const float speedModifer);
    void terimateAnimation(const InstanceID id, const std::string& name);

private:

    void setupGraphicsState();

    GLFWwindow* mWindow;

    bool mFirstFrame = true;
    bool mShouldClose = false;

    Level* mCurrentLevel;

    std::filesystem::path mRootDir;
    Scene* mScene;
    RenderEngine* mRenderEngine;
    RenderThread* mRenderThread;
    PhysicsWorld* mPhysicsEngine;
    ScriptEngine* mScriptEngine;

};

}

#endif
