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
    class Player;
    class Controller;

class TempestEngine
{
public:
    TempestEngine(GLFWwindow* window, const std::filesystem::path& rootDir);
    ~TempestEngine();

    // Load level
    void loadLevel(const std::filesystem::path& path);

    // main loop to be called once c++ side.
    void run();

    // lua scripting hooks.
    // must be called before updating transformation!!
    void startInstanceFrame(const InstanceID);

    void setInstanceLinearVelocity(const InstanceID, const float3&);
    void translateInstance(const InstanceID, const float3&);
    float3 getInstancePosition(const InstanceID) const;
    void   setInstancePosition(const InstanceID, const float3&);
    void   setInstanceRotation(const InstanceID, const quat&);
    void   setGraphicsInstancePosition(const InstanceID, const float3&);
    float3 getInstanceSize(const InstanceID) const;
    float3 getInstanceCenter(const InstanceID) const;

    float3 getPhysicsBodyPosition(const InstanceID);

    void updatePlayersAttachedCameras(const InstanceID);

    void startAnimation(const InstanceID id, const std::string& name, const bool loop, const float speedModifer);
    void terminateAnimation(const InstanceID id, const std::string& name);

    InstanceID getInstanceIDByName(const std::string&) const;
    SceneID getSceneIDByName(const std::string&) const;

    void setMainCameraByName(const std::string&);
    void setShadowCameraByName(const std::string&);

    void createPlayerInstance(const InstanceID, const float3& pos, const float3& dir);
    const Controller& getControllerForInstance(const InstanceID);
    void createControllerInstance(const InstanceID, const uint32_t);
    const Controller& updateControllerInstance(const InstanceID);
    void attachCameraToPlayer(const InstanceID id, const std::string&, const float armatureLenght);
    void attachShadowCameraToPlayer(const InstanceID id, const std::string&);

    void applyImpulseToInstance(const InstanceID, const float3&);

    float3 getCameraDirectionByName(const std::string&) const;
    float3 getCameraRightByName(const std::string&) const;
    float3 getCameraPositionByName(const std::string&) const;

private:

    void setupGraphicsState();

    GLFWwindow* mWindow;

    bool mFirstFrame = true;
    bool mShouldClose = false;

    Level* mCurrentLevel;

    std::unordered_map<InstanceID, std::unique_ptr<Player>> mPlayers;
    std::unordered_map<InstanceID, std::unique_ptr<Controller>> mControllers;

    std::filesystem::path mRootDir;
    RenderEngine* mRenderEngine;
    RenderThread* mRenderThread;
    PhysicsWorld* mPhysicsEngine;
    ScriptEngine* mScriptEngine;

};

}

#endif
