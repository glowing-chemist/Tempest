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
    class SceneWindow;
    class InstanceWindow;

class Level
{
public:
    Level(RenderEngine* eng,
          PhysicsWorld* physWorld,
          ScriptEngine*,
          const std::filesystem::path& path,
          InstanceWindow* instanceWindow = nullptr,
          SceneWindow* sceneWindow = nullptr);

    Level(RenderEngine* eng,
          PhysicsWorld* physWorld,
          ScriptEngine*,
          const std::filesystem::path& path,
          const std::string& name,
          InstanceWindow* instanceWindow = nullptr,
          SceneWindow* sceneWindow = nullptr);

    const std::filesystem::path& getWorkingDirectory() const
    {
        return mWorkingDir;
    }

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

    Camera& getCameraByName(const std::string& n)
    {
        BELL_ASSERT(mCamera.find(n) != mCamera.end(), "Camera not created")
        if(auto it = mCamera.find(n); it != mCamera.end())
        {
            return it->second;
        }

        BELL_TRAP;
    }

    const std::unordered_map<std::string, SceneID>& getAssets() const
    {
        return mAssetIDs;
    }

    std::string getAssetPath(const SceneID id)
    {
        return mIDToPath[id];
    }

    const std::unordered_map<std::string, InstanceID> getInstances() const
    {
        return mInstanceIDs;
    }

    const std::unordered_map<std::string, Camera> getCameras() const
    {
        return mCamera;
    }

    std::string getAssetName(const SceneID id)
    {
        return mAssetNames[id];
    }

    std::vector<std::string> getMaterials() const
    {
        std::vector<std::string> materialNames{};
        materialNames.reserve(mMaterials.size());
        for(const auto&[name, entry] : mMaterials)
            materialNames.push_back(name);

        return materialNames;
    }

    struct MaterialEntry
    {
        MaterialEntry() :
                mMaterialOffset{0},
                mMaterialFlags{0} {}

        MaterialEntry(const uint32_t o, const uint32_t f) :
                mMaterialOffset{o},
                mMaterialFlags{f} {}

        uint32_t mMaterialOffset;
        uint32_t mMaterialFlags;

        std::string mAlbedoPath;
        std::string mMetalnessPath;
        std::string mRoughnessPath;
        std::string mNormalPath;
        std::string mEmissivePath;
        std::string mOcclusionPath;
    };

    const std::unordered_map<std::string, MaterialEntry>& getMaterialEntries() const
    {
        return mMaterials;
    }

    const std::string& getMaterialName(const InstanceID id) const
    {
        if(auto it = mInstanceMapertials.find(id); it != mInstanceMapertials.end())
            return it->second;
        else
            return nullptr;
    }

    void addMeshFromFile(const std::filesystem::path& path, const MeshType);
    void addMeshInstance(const std::string& name, const SceneID, const std::string& materialsName, const float3& pos,
                         const quat& rotation, const float3& scale);

    void setInstanceMaterial(const InstanceID id, const std::string& n)
    {
        mInstanceMapertials[id] = n;
        MeshInstance* instance = mScene->getMeshInstance(id);
        MaterialEntry entry = mMaterials[n];
        instance->setMaterialIndex(entry.mMaterialOffset);
        instance->setMaterialFlags(entry.mMaterialFlags);
    }

    const std::array<std::string, 6>& getSkybox() const
    {
        return mSkybox;
    }

private:

    void addMesh(const std::string& name, const Json::Value& entry);
    void addMeshInstance(const std::string& name, const Json::Value& entry);
    void addLight(const std::string& name, const Json::Value& entry);
    void addMaterial(const std::string& name, const Json::Value& entry);
    void addScript(const std::string& name, const Json::Value& entry);
    void addCamera(const std::string& name, const Json::Value& entry);
    void processGlobals(const std::string& name, const Json::Value& entry);

    std::string mName;
    std::filesystem::path mWorkingDir;

    std::unordered_map<std::string, Camera>  mCamera;
    std::unordered_map<std::string, SceneID> mAssetIDs;
    std::unordered_map<SceneID, std::string> mAssetNames;
    std::unordered_map<SceneID, std::string> mIDToPath;
    std::unordered_map<std::string, InstanceID> mInstanceIDs;
    std::unordered_map<InstanceID, std::string> mInstanceMapertials;
    std::unordered_map<std::string, MaterialEntry> mMaterials;

    std::unique_ptr<Scene> mScene;
    RenderEngine* mRenderEngine;
    PhysicsWorld* mPhysWorld;
    ScriptEngine* mScriptEngine;

    std::array<std::string, 6> mSkybox;
    std::vector<std::string> mGlobalScripts;

    // Used to hooks in the editor.
    SceneWindow* mSceneWindow;
    InstanceWindow* mInstanceWindow;
};

}

#endif
