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


void Level::addMesh(const std::string& name, const Json::Value& entry)
{
    BELL_ASSERT(entry.isMember("Path") && entry.isMember("Dynamism"), "Fields rewuired for mesh")
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

        float3 collisderScale = {1.0f, 1.0f, 1.0f};
        if(colliderEntry.isMember("Scale"))
        {
            const Json::Value &scaleEntry = colliderEntry["Scale"];
            BELL_ASSERT(scaleEntry.isArray(), "Scale not correct format")
            collisderScale.x = scaleEntry[0].asFloat();
            collisderScale.y = scaleEntry[1].asFloat();
            collisderScale.z = scaleEntry[2].asFloat();
        }

        float mass = 0.f;
        if(colliderEntry.isMember("Mass"))
        {
            mass = colliderEntry["Mass"].asFloat();
        }

        mPhysWorld->addObject(id, entityType, colliderType, position, collisderScale, mass);
    }

    if(entry.isMember("Scripts"))
    {
        const Json::Value& scriptEntry = entry["Scripts"];
        if(scriptEntry.isMember("GamePlay"))
        {
            const Json::Value& gamePlayScript = scriptEntry["GamePlay"];
            const std::string func = gamePlayScript.asString();
            mScriptEngine->registerEntityWithScript(func, id, ScriptContext::kContext_GamePlay);
        }

        if(scriptEntry.isMember("Graphics"))
        {
            const Json::Value& graphicscript = scriptEntry["Graphics"];
            const std::string func = graphicscript.asString();

            mScriptEngine->registerEntityWithScript(func, id, ScriptContext::kContext_Graphics);
        }

        if(scriptEntry.isMember("Physics"))
        {
            const Json::Value& physicsScript = scriptEntry["Physics"];
            const std::string func = physicsScript.asString();

            mScriptEngine->registerEntityWithScript(func, id, ScriptContext::kContext_Physics);
        }
    }

}


void Level::addLight(const std::string& name, const Json::Value& entry)
{
    BELL_TRAP;
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

    mMaterials[name] = {matPaths.mMaterialOffset, matPaths.mMaterialTypes};

    mScene->addMaterial(matPaths, mRenderEngine);
}

void Level::addScript(const std::string &name, const Json::Value &entry)
{
    std::string scriptPath{};
    if(entry.isMember("Path"))
    {
        scriptPath = (mWorkingDir / entry["Path"].asString()).string();
    }

    ScriptContext context = ScriptContext::kContext_GamePlay;
    if(entry.isMember("Context"))
    {
        const std::string contextString = entry["Context"].asString();
        if(contextString == "GamePlay")
            context = ScriptContext::kContext_GamePlay;
        else if(contextString == "Graphics")
            context = ScriptContext::kContext_Graphics;
        else if(contextString == "Physics")
            context = ScriptContext::kContext_Physics;
    }

    BELL_ASSERT(!scriptPath.empty(), "No path given for script")
    mScriptEngine->registerScript(scriptPath, name, context);
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


}
