#include "Level.hpp"
#include "Engine/Engine.hpp"
#include "Engine/GeomUtils.h"

#include "PhysicsWorld.hpp"
#include "ScriptEngine.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>

namespace Tempest
{

Level::Level(RenderEngine *eng, PhysicsWorld* physWorld, ScriptEngine* scriptEngine, const std::filesystem::path& path, const std::string& name) :
        mName(name),
        mWorkingDir(path.parent_path().string()),
        mScene(new Scene(path)),
        mRenderEngine(eng),
        mPhysWorld{physWorld},
        mScriptEngine{scriptEngine}
{
    std::ifstream sceneFile;
    sceneFile.open(path);
    BELL_ASSERT(sceneFile.is_open(), "Failed to open scene file")

    Json::Value sceneRoot;
    sceneFile >> sceneRoot;

    std::array<std::string, 7> sections{"GLOBALS", "MESH", "MATERIALS", "INSTANCE", "LIGHT",
                                        "CAMERA", "SCRIPTS"};
    std::array<void(Level::*)(const std::string&, const Json::Value&), 7> sectionFunctions
        {
            &Level::processGlobals, &Level::addMesh, &Level::addMaterial, &Level::addMeshInstance,
            &Level::addLight, &Level::addCamera, &Level::addScript
        };

    for(uint32_t i = 0; i < sections.size(); ++i)
    {
        if(sceneRoot.isMember(sections[i]))
        {
            for(std::string& entityName : sceneRoot[sections[i]].getMemberNames())
                std::invoke(sectionFunctions[i], this, entityName, sceneRoot[sections[i]][entityName]);
        }

    }

    mScene->computeBounds(AccelerationStructure::Dynamic);
    mScene->computeBounds(AccelerationStructure::Static);
}


Level::Level(RenderEngine* eng, PhysicsWorld* physWorld, ScriptEngine* scriptEngine, const std::string& name) :
        mName(name),
        mWorkingDir("."),
        mScene(new Scene(name)),
        mRenderEngine(eng),
        mPhysWorld{physWorld},
        mScriptEngine{scriptEngine}
{

}


void Level::addMesh(const std::string& name, const Json::Value& entry)
{
    BELL_ASSERT(entry.isMember("Path") && entry.isMember("Dynamism"), "Fields required for mesh")
    const std::string path = entry["Path"].asString();
    const std::string type = entry["Dynamism"].asString();

    const std::vector<SceneID> ids = mScene->loadFile((mWorkingDir / path).string(), type == "Dynamic" ? MeshType::Dynamic : MeshType::Static, mRenderEngine);
    for(uint32_t i = 0; i < ids.size(); ++i)
    {
        const SceneID id = ids[i];
        if(ids.size() > 1)
            mAssetIDs[name + std::to_string(i)] = id;
        else
            mAssetIDs[name] = id;

        mIDToPath[id] = path;
    }
}


void Level::addMeshInstance(const std::string& name, const Json::Value& entry)
{
    const std::string assetName = entry["Asset"].asString();
    BELL_ASSERT(mAssetIDs.find(assetName) != mAssetIDs.end(), "Unable to find asset")
    const SceneID assetID = mAssetIDs[assetName];

    float3 position{0.0f, 0.0f, 0.0f};
    float3 scale{1.0f, 1.0f, 1.0f};
    quat   rotation{1.0f, 0.0f, 0.0f, 0.f};
    uint32_t materialOffset = 0;
    uint32_t materialFlags = 0;

    if(entry.isMember("Position"))
    {
        const Json::Value& positionEntry = entry["Position"];
        BELL_ASSERT(positionEntry.isArray(), "Position not correct format")
        position.x = positionEntry[0].asFloat();
        position.y = positionEntry[1].asFloat();
        position.z = positionEntry[2].asFloat();
    }

    if(entry.isMember("Scale"))
    {
        const Json::Value& scaleEntry = entry["Scale"];
        BELL_ASSERT(scaleEntry.isArray(), "Scale not correct format")
        scale.x = scaleEntry[0].asFloat();
        scale.y = scaleEntry[1].asFloat();
        scale.z = scaleEntry[2].asFloat();
    }

    if(entry.isMember("Rotation"))
    {
        const Json::Value& rotationEntry = entry["Rotation"];
        BELL_ASSERT(rotationEntry.isArray(), "Rotation not correct format")
        rotation.x = rotationEntry[0].asFloat();
        rotation.y = rotationEntry[1].asFloat();
        rotation.z = rotationEntry[2].asFloat();
        rotation.w = rotationEntry[3].asFloat();
        rotation = glm::normalize(rotation);
    }

    BELL_ASSERT(entry.isMember("Material"), "Material is a required field")
    if(entry.isMember("Material"))
    {
        const std::string materialName = entry["Material"].asString();
        BELL_ASSERT(mMaterials.find(materialName) != mMaterials.end(), "Using unspecified material")
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
    mInstanceMapertials[id] = entry["Material"].asString();

    // Now check for collision geometry
    if(entry.isMember("Collider"))
    {
        const Json::Value& colliderEntry = entry["Collider"];

        BasicCollisionGeometry colliderType = BasicCollisionGeometry::Box;

        if(colliderEntry.isMember("Geometry"))
        {
            const std::string type = colliderEntry["Geometry"].asString();
            if(type == "MESH")
            {
                BELL_TRAP; // TODO
            }
            else if(type == "Capsule")
                colliderType = BasicCollisionGeometry::Capsule;
            else if(type == "Box")
                colliderType = BasicCollisionGeometry::Box;
            else if(type == "Plane")
                colliderType = BasicCollisionGeometry::Plane;
            else if(type == "Sphere")
                colliderType = BasicCollisionGeometry::Sphere;
            else
            {
                BELL_TRAP;
            }
        }

        PhysicsEntityType entityType = PhysicsEntityType::StaticRigid;
        if(colliderEntry.isMember("Type"))
        {
            const std::string type = colliderEntry["Type"].asString();
            if(type == "Kinematic")
                entityType = PhysicsEntityType::Kinematic;
            else if(type == "Static")
                entityType = PhysicsEntityType::StaticRigid;
            else if(type == "Dynamic")
                entityType = PhysicsEntityType::DynamicRigid;
            else
            {
                BELL_TRAP;
            }
        }

        float3 collisderScale;
        if(colliderEntry.isMember("Scale"))
        {
            const Json::Value &scaleEntry = colliderEntry["Scale"];
            BELL_ASSERT(scaleEntry.isArray(), "Scale not correct format")
            collisderScale.x = scaleEntry[0].asFloat();
            collisderScale.y = scaleEntry[1].asFloat();
            collisderScale.z = scaleEntry[2].asFloat();
        }
        else
        {
            const StaticMesh* assetMesh = mScene->getMesh(assetID);
            AABB bounds = assetMesh->getAABB();
            float3 boundsSize = bounds.getSideLengths();
            collisderScale = scale * boundsSize;
        }

        float mass = 0.f;
        if(colliderEntry.isMember("Mass"))
        {
            mass = colliderEntry["Mass"].asFloat();
        }

        mPhysWorld->addObject(id, entityType, colliderType, position, rotation, collisderScale, mass);
    }

    if(entry.isMember("Scripts"))
    {
        const Json::Value& scriptEntry = entry["Scripts"];
        if(scriptEntry.isMember("GamePlay"))
        {
            const Json::Value& gamePlayScript = scriptEntry["GamePlay"];
            const std::string func = gamePlayScript.asString();
            mScriptEngine->registerEntityWithScript(func, id);
        }
    }

}


void Level::addLight(const std::string& name, const Json::Value& entry)
{
    float4 position = {0, 0, 0., 1.0f};
    float4 direction{1.0f, 0.0f, 0.0f, 1.0f};
    float4 up{0.f, 1.0f, 0.0f, 1.0f};
    float4 colour{1.0f, 1.0f, 1.0f, 1.0f};
    float2 size{1.0f, 1.0f};
    float intensity = 1.0f;
    float radius = 1.0f;
    if(entry.isMember("Position"))
    {
        const Json::Value& positionEntry = entry["Position"];
        BELL_ASSERT(positionEntry.isArray(), "Position not correct format")
        position.x = positionEntry[0].asFloat();
        position.y = positionEntry[1].asFloat();
        position.z = positionEntry[2].asFloat();
    }

    if(entry.isMember("Direction"))
    {
        const Json::Value& directionEntry = entry["Direction"];
        BELL_ASSERT(directionEntry.isArray(), "Direction not correct format")
        direction.x = directionEntry[0].asFloat();
        direction.y = directionEntry[1].asFloat();
        direction.z = directionEntry[2].asFloat();
    }

    if(entry.isMember("FallOff"))
        radius = entry["FallOff"].asFloat();

    if(entry.isMember("Intensity"))
        intensity = entry["Intensity"].asFloat();

    if(entry.isMember("Colour"))
    {
        const Json::Value& colourEntry = entry["Colour"];
        BELL_ASSERT(colourEntry.isArray(), "Colour not correct format")
        colour.x = colourEntry[0].asFloat();
        colour.y = colourEntry[1].asFloat();
        colour.z = colourEntry[2].asFloat();
    }

    if(entry.isMember("Size"))
    {
        const Json::Value& sizeEntry = entry["Size"];
        BELL_ASSERT(sizeEntry.isArray(), "Size not correct format")
        size.x = sizeEntry[0].asFloat();
        size.y = sizeEntry[1].asFloat();
    }

    BELL_ASSERT(entry.isMember("Type"), "Light must specify type")
    const std::string lightType = entry["Type"].asString();
    if(lightType == "Point")
    {
        mScene->addLight(Scene::Light::pointLight(position, colour, intensity, radius));
    }
    else if(lightType == "Spot")
    {
        mScene->addLight(Scene::Light::spotLight(position, direction, colour, intensity, radius, 45.0f));
    }
    else if(lightType == "Area")
    {
        mScene->addLight(Scene::Light::areaLight(position, direction, up, colour, intensity, radius, size));
    }
}

void Level::addMaterial(const std::string &name, const Json::Value &entry)
{
    Scene::MaterialPaths matPaths{};
    matPaths.mMaterialOffset = mScene->getMaterials().size();

    if(entry.isMember("Albedo"))
    {
        const std::string path = entry["Albedo"].asString();
        matPaths.mAlbedoorDiffusePath = (mWorkingDir / path).string();
        matPaths.mMaterialTypes |= static_cast<uint32_t>(MaterialType::Albedo);
    }

    if(entry.isMember("Normal"))
    {
        const std::string path = entry["Normal"].asString();
        matPaths.mNormalsPath =  (mWorkingDir / path).string();
        matPaths.mMaterialTypes |= static_cast<uint32_t>(MaterialType::Normals);
    }

    if(entry.isMember("Roughness"))
    {
        const std::string path = entry["Roughness"].asString();
        matPaths.mRoughnessOrGlossPath = (mWorkingDir / path).string();
        matPaths.mMaterialTypes |= static_cast<uint32_t>(MaterialType::Roughness);
    }

    if(entry.isMember("Metalness"))
    {
        const std::string path = entry["Metalness"].asString();
        matPaths.mMetalnessOrSpecularPath = (mWorkingDir / path).string();
        matPaths.mMaterialTypes |= static_cast<uint32_t>(MaterialType::Metalness);
    }

    if(entry.isMember("MetalnessRoughness"))
    {
        const std::string path = entry["MetalnessRoughness"].asString();
        matPaths.mRoughnessOrGlossPath = (mWorkingDir / path).string();
        matPaths.mMaterialTypes |= static_cast<uint32_t>(MaterialType::CombinedMetalnessRoughness);
    }

    if(entry.isMember("Emissive"))
    {
        const std::string path = entry["Emissive"].asString();
        matPaths.mEmissivePath = (mWorkingDir / path).string();
        matPaths.mMaterialTypes |= static_cast<uint32_t>(MaterialType::Emisive);
    }

    if(entry.isMember("Occlusion"))
    {
        const std::string path = entry["Occlusion"].asString();
        matPaths.mAmbientOcclusionPath = (mWorkingDir / path).string();
        matPaths.mMaterialTypes |= static_cast<uint32_t>(MaterialType::AmbientOcclusion);
    }

    mMaterials[name] = MaterialEntry{matPaths.mMaterialOffset, matPaths.mMaterialTypes};

    mScene->addMaterial(matPaths, mRenderEngine);
}

void Level::addScript(const std::string &name, const Json::Value &entry)
{
    std::string scriptPath = (mWorkingDir / entry.asString()).string();

    BELL_ASSERT(!scriptPath.empty(), "No path given for script")
    mScriptEngine->registerScript(scriptPath, name);
}


void Level::addCamera(const std::string& name, const Json::Value& entry)
{
    Camera newCamera({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 1.0f);
    if(entry.isMember("Position"))
    {
        const Json::Value& positionEntry = entry["Position"];
        float3 position;
        position.x = positionEntry[0].asFloat();
        position.y = positionEntry[1].asFloat();
        position.z = positionEntry[2].asFloat();

        newCamera.setPosition(position);
    }

    if(entry.isMember("Direction"))
    {
        const Json::Value& directionEntry = entry["Direction"];
        float3 direction;
        direction.x = directionEntry[0].asFloat();
        direction.y = directionEntry[1].asFloat();
        direction.z = directionEntry[2].asFloat();
        direction = glm::normalize(direction);

        newCamera.setDirection(direction);
    }

    if(entry.isMember("Aspect"))
    {
        const float aspect = entry["Aspect"].asFloat();
        newCamera.setAspect(aspect);
    }

    if(entry.isMember("NearPlane"))
    {
        const float NearPlane = entry["NearPlane"].asFloat();
        newCamera.setNearPlane(NearPlane);
    }

    if(entry.isMember("FarPlane"))
    {
        const float FarPlane = entry["FarPlane"].asFloat();
        newCamera.setFarPlane(FarPlane);
    }

    if(entry.isMember("FOV"))
    {
        const float fov = entry["FOV"].asFloat();
        newCamera.setFOVDegrees(fov);
    }

    if(entry.isMember("Mode"))
    {
        const std::string mode = entry["Mode"].asString();
        CameraMode Cmode = CameraMode::Perspective;
        if(mode == "Perspective")
            Cmode = CameraMode::Perspective;
        else if (mode == "Orthographic")
            Cmode = CameraMode::Orthographic;
        else if(mode == "InfinitePerspective")
            Cmode = CameraMode::InfinitePerspective;

        newCamera.setMode(Cmode);
    }

    if(entry.isMember("OrthoSize"))
    {
        const Json::Value& size = entry["OrthoSize"];
        float2 s;
        s.x = size[0].asFloat();
        s.y = size[1].asFloat();

        newCamera.setOrthographicSize(s);
    }

    mCamera.insert({name, newCamera});
}


void Level::processGlobals(const std::string& name, const Json::Value& entry)
{
    if(entry.isMember("Skybox"))
    {
        BELL_ASSERT(entry["Skybox"].isArray(), "Skybox must be array")
        const Json::Value skyboxes = entry["Skybox"];
        std::array<std::string, 6> skyboxPaths{};
        for(uint32_t i = 0; i < 6; ++i)
        {
            skyboxPaths[i] = (mWorkingDir / skyboxes[i].asString()).string();
        }

        mScene->loadSkybox(skyboxPaths, mRenderEngine);
    }

    if(entry.isMember("ShadowMapRes"))
    {
        const Json::Value& shadowMapRes = entry["ShadowMapRes"];
        float2 res;
        res.x = shadowMapRes[0].asFloat();
        res.y = shadowMapRes[1].asFloat();

        mRenderEngine->setShadowMapResolution(res);
    }

    if(entry.isMember("Scripts"))
    {
        const Json::Value& scripts = entry["Scripts"];
        for(uint32_t i = 0; i < scripts.size(); ++i)
        {
            const std::string scriptPath = (mWorkingDir / scripts[i].asString()).string();
            mScriptEngine->loadScript(scriptPath);
        }
    }
}

void Level::setMainCameraByName(const std::string& name)
{
    BELL_ASSERT(mCamera.find(name) != mCamera.end(), "Camera has not been created")
    Camera& cam = mCamera.find(name)->second;

    mScene->setCamera(&cam);
}


void Level::setShadowCameraByName(const std::string& name)
{
    BELL_ASSERT(mCamera.find(name) != mCamera.end(), "Camera has not been created")
    Camera& cam = mCamera.find(name)->second;

    mScene->setShadowingLight(&cam);
}


void Level::addMaterialFromFile(std::filesystem::path& materialFile)
{

}


void Level::addMeshFromFile(std::filesystem::path& path, const MeshType)
{

}


void Level::addScriptFromFile(std::filesystem::path& path)
{

}

}
