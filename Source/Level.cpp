#include "Level.hpp"

#include <fstream>

namespace Tempest
{

Level::Level(Engine *eng, const std::filesystem::path& path, const std::string& name) :
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

}


void Level::addLight(const std::string& name, const Json::Value& entry)
{

}

}
