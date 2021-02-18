#include "Level.hpp"
#include "Engine/GeomUtils.h"

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>

namespace Tempest
{

Level::Level(RenderEngine *eng, const std::filesystem::path& path, const std::string& name) :
    mName(name),
    mScene(new Scene(path)),
    mEngine(eng)
{
    std::ifstream sceneFile;
    sceneFile.open(path);

    Json::Value sceneRoot;
    sceneFile >> sceneRoot;

    for(const std::string& entityName : sceneRoot.getMemberNames())
    {
        Json::Value entity = sceneRoot[entityName];
        const Json::Value type = entity["Type"];

        if(type.asString() == "MESH")
        {
            addMesh(entityName, entity);
        }
        else if(type.asString() == "INSTANCE")
        {
            addMeshInstance(entityName, entity);
        }
        else if(type.asString() == "LIGHT")
        {
            addLight(entityName, entity);
        }
        else if(type.asString() == "MATERIAL")
        {
            addMaterial(entityName, entity);
        }
        else if(type.asString() == "SCRIPT")
        {
            addScript(entityName, entity);
        }
        else
        {
            BELL_TRAP; // unknown type
        }
    }
}


void Level::addMesh(const std::string& name, const Json::Value& entry)
{
    const std::string path = entry["Path"].asString();
    const std::string type = entry["Dynamism"].asString();

    const std::vector<SceneID> ids = mScene->loadFile(path, type == "Dynamic" ? MeshType::Dynamic : MeshType::Static, mEngine);
    for(uint32_t i = 0; i < ids.size(); ++i)
    {
        const SceneID id = ids[i];
        mAssetIDs[name + std::to_string(i)] = id;
    }
}


void Level::addMeshInstance(const std::string& name, const Json::Value& entry)
{
    const std::string assetName = entry["Asset"].asString();
    BELL_ASSERT(mAssetIDs.find(assetName) != mAssetIDs.end(), "Unable to find asset")
    const SceneID assetID = mAssetIDs[assetName];

    float3 position{0.0f, 0.0f, 0.0f};
    float3 scale{1.0f, 1.0f, 1.0f};
    quat   rotation{0.0f, 0.0f, 0.0f, 0.f};
    uint32_t materialOffset = 0;
    uint32_t materialFlags = 0;

    if(entry.isMember("Positon"))
    {
        const Json::Value& positionEntry = entry["position"];
        BELL_ASSERT(positionEntry.isArray(), "Position not correct format")
        position.x = positionEntry.get("x", 0.0f).asFloat();
        position.y = positionEntry.get("y", 0.0f).asFloat();
        position.z = positionEntry.get("z", 0.0f).asFloat();
    }

    if(entry.isMember("Scale"))
    {
        const Json::Value& scaleEntry = entry["Scale"];
        BELL_ASSERT(scaleEntry.isArray(), "Scale not correct format")
        scale.x = scaleEntry.get("x", 0.0f).asFloat();
        scale.y = scaleEntry.get("y", 0.0f).asFloat();
        scale.z = scaleEntry.get("z", 0.0f).asFloat();
    }

    if(entry.isMember("Rotation"))
    {
        const Json::Value& rotationEntry = entry["Rotation"];
        BELL_ASSERT(rotationEntry.isArray(), "Rotation not correct format")
        rotation.x = rotationEntry.get("x", 0.0f).asFloat();
        rotation.y = rotationEntry.get("y", 0.0f).asFloat();
        rotation.z = rotationEntry.get("z", 0.0f).asFloat();
        rotation.w = rotationEntry.get("w", 0.0f).asFloat();
    }

    BELL_ASSERT(entry.isMember("Material"), "Material is a required field")
    if(entry.isMember("Material"))
    {
        const std::string materialName = entry["MaterialIndex"].asString();
        BELL_ASSERT(mMaterials.find(naterialName) != mMaterials.end(), "Using unspecified material")
        const MaterialEntry material = mMaterials[materialName];
        materialOffset = material.mMaterialOffset;
        materialFlags = material.mMaterialFlags;
    }

    const float4x4 transform = glm::translate(float4x4(1.0f), position) *
                                            glm::scale(float4x4(1.0f), scale) *
                                            glm::mat4_cast(rotation);

    const InstanceID id = mScene->addMeshInstance(assetID,
                                                  kInvalidInstanceID,
                                                  transform,
                                                  materialOffset,
                                                  materialFlags,
                                                  name);

    mInstanceIDs[name] = id;
}


void Level::addLight(const std::string& name, const Json::Value& entry)
{

}

void Level::addMaterial(const std::string &name, const Json::Value &entry)
{

}

void Level::addScript(const std::string &name, const Json::Value &entry)
{

}

}
