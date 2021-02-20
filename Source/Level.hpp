#ifndef TEMPEST_LEVEL_HPP
#define TEMPEST_LEVEL_HPP

#include <filesystem>
#include <string>
#include <memory>
#include <unordered_map>

#include "json/json.h"

#include "Engine/Scene.h"

namespace Tempest
{

    class PhysicsWorld;
    class ScriptEngine;

class Level
{
public:
    Level(RenderEngine* eng, PhysicsWorld* physWorld, ScriptEngine*, const std::filesystem::path& path, const std::string& name);

    Scene* getScene()
    {
        return mScene.get();
    }

    const std::string& getName() const
    {
        return mName;
    }

    InstanceID getInstanceIDByName(const std::string& name) const
    {
        if(auto it = mInstanceIDs.find(name); it != mInstanceIDs.end())
            return it->second;
        else
            return kInvalidInstanceID;
    }
    SceneID    getSceneIDByname(const std::string& name) const
    {
        if(auto it = mAssetIDs.find(name); it != mAssetIDs.end())
            return it->second;
        else
            return kInvalidInstanceID;
    }

    void setMainCameraByName(const std::string&);
    void setShadowCameraByName(const std::string&);

private:

    void addMesh(const std::string& name, const Json::Value& entry);
    void addMeshInstance(const std::string& name, const Json::Value& entry);
    void addLight(const std::string& name, const Json::Value& entry);
    void addMaterial(const std::string& name, const Json::Value& entry);
    void addScript(const std::string& name, const Json::Value& entry);
    void addCamera(const std::string& name, const Json::Value& entry);
    void processGlobals(const std::string& name, const Json::Value& entry);

    std::string mName;

    struct MaterialEntry
    {
        uint32_t mMaterialOffset;
        uint32_t mMaterialFlags;
    };

    std::unordered_map<std::string, Camera>  mCamera;
    std::unordered_map<std::string, SceneID> mAssetIDs;
    std::unordered_map<std::string, SceneID> mInstanceIDs;
    std::unordered_map<std::string, MaterialEntry> mMaterials;

    std::unique_ptr<Scene> mScene;
    RenderEngine* mRenderEngine;
    PhysicsWorld* mPhysWorld;
    ScriptEngine* mScriptEngine;
};

}

#endif
