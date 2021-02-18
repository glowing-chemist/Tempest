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

class Level
{
public:
    Level(RenderEngine* eng, const std::filesystem::path& path, const std::string& name);

    Scene* getScene()
    {
        return mScene.get();
    }

    const std::string& getName() const
    {
        return mName;
    }

private:

    void addMesh(const std::string& name, const Json::Value& entry);
    void addMeshInstance(const std::string& name, const Json::Value& entry);
    void addLight(const std::string& name, const Json::Value& entry);
    void addMaterial(const std::string& name, const Json::Value& entry);
    void addScript(const std::string& name, const Json::Value& entry);

    std::string mName;

    struct MaterialEntry
    {
        uint32_t mMaterialOffset;
        uint32_t mMaterialFlags;
    };

    std::unordered_map<std::string, SceneID> mAssetIDs;
    std::unordered_map<std::string, SceneID> mInstanceIDs;
    std::unordered_map<std::string, MaterialEntry> mMaterials;

    std::unique_ptr<Scene> mScene;
    RenderEngine* mEngine;
};

}

#endif
