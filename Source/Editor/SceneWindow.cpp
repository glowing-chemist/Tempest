#include "SceneWindow.hpp"
#include "InstanceWindow.hpp"
#include "../Level.hpp"

#include "imgui.h"

#include <fstream>

namespace Tempest
{
    SceneWindow::SceneWindow() :
        mCurrentLevel{nullptr}
    {
        std::memset(mInstanceAddTextEdit, 0, 64);
    }

    bool SceneWindow::renderUI()
    {
        bool changedScene = false;

        if(mCurrentLevel)
        {
            if (ImGui::Begin("Scene"))
            {
                if(ImGui::TreeNode("Assets"))
                {
                    const std::unordered_map<std::string, SceneID>& assets = mCurrentLevel->getAssets();
                    for(const auto& [name, id] : assets)
                    {
                        ImGui::Text("Mesh: %s, ID: %d", name.c_str(), id);
                        ImGui::SameLine();
                        ImGui::PushID(id);
                        if(ImGui::Button("Add instance"))
                        {
                            mShowInstanceAddWindow = true;
                            mAssetToAdd = id;
                        }
                        ImGui::SameLine();
                        bool dynamism = mAssetDynamism[id];
                        ImGui::Checkbox("Dynamic", &dynamism);
                        mAssetDynamism[id] = dynamism;
                        ImGui::PopID();

                    }

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("Instances"))
                {
                    mSelected.clear();

                    const std::unordered_map<std::string, InstanceID>& instances = mCurrentLevel->getInstances();
                    for(const auto& [name, id] : instances)
                    {
                        if(ImGui::TreeNode(name.c_str()))
                        {
                            mSelected.push_back(id);
                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("Cameras"))
                {
                    const std::unordered_map<std::string, Camera>& cameras = mCurrentLevel->getCameras();
                    for(const auto& [name, cam] : cameras)
                    {
                        if(ImGui::TreeNode(name.c_str()))
                        {
                            const float3& position = cam.getPosition();
                            const float3& direction = cam.getDirection();
                            ImGui::Text("Position %f %f %f", position.x, position.y, position.z);
                            ImGui::Text("Direction %f %f %f", direction.x, direction.y, direction.z);
                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }

                ImGui::End();
            }
        }

        if(mShowInstanceAddWindow)
            changedScene = drawInstanceAddWindow();

        return changedScene;
    }

    void SceneWindow::exportSceneToFile(const InstanceWindow* instanceWindow)
    {
        Json::Value root{};

        // Export materials
        {
            Json::Value materialsJson{};
            const std::unordered_map<std::string, Level::MaterialEntry>& materials = mCurrentLevel->getMaterialEntries();
            for(const auto&[name, entries] : materials)
            {
                Json::Value material{};
                if(entries.mMaterialFlags & MaterialType::Albedo)
                    material["Albedo"] = entries.mAlbedoPath;
                if(entries.mMaterialFlags & MaterialType::Normals)
                    material["Normal"] = entries.mNormalPath;
                if(entries.mMaterialFlags & MaterialType::Roughness)
                    material["Roughness"] = entries.mRoughnessPath;
                if(entries.mMaterialFlags & MaterialType::Metalness)
                    material["Metalness"] = entries.mMetalnessPath;
                if(entries.mMaterialFlags & MaterialType::CombinedMetalnessRoughness)
                    material["MetalnessRoughness"] = entries.mRoughnessPath;
                if(entries.mMaterialFlags & MaterialType::Emisive)
                    material["Emissive"] = entries.mEmissivePath;
                if(entries.mMaterialFlags & MaterialType::AmbientOcclusion)
                    material["Occlusion"] = entries.mOcclusionPath;

                materialsJson[name] = material;
            }

            root["MATERIALS"] = materialsJson;
        }

        // Export assets
        {
            Json::Value meshJson{};
            const std::unordered_map<std::string, SceneID>& assets = mCurrentLevel->getAssets();
            for(const auto&[name, id] : assets)
            {
                const bool isDynamic = mAssetDynamism[id];
                const std::string path = mCurrentLevel->getAssetPath(id);
                 Json::Value mesh{};

                 mesh["Path"] = std::filesystem::path(path).lexically_relative(mCurrentLevel->getWorkingDirectory()).string();
                 mesh["Dynamism"] = isDynamic ? "Dynamic" : "Static";

                meshJson[name] = mesh;
            }

            root["MESH"] = meshJson;
        }

        // Export instances
        {
            Json::Value instancesJson{};
            std::unordered_map<InstanceID, InstanceWindow::InstanceEntry> info = instanceWindow->getInstanceInfo();
            const std::unordered_map<std::string, InstanceID>& instances = mCurrentLevel->getInstances();
            for(const auto&[name, id] : instances)
            {
                Json::Value instanceJSon{};

                InstanceWindow::InstanceEntry entry = info[id];
                const MeshInstance* instance = mCurrentLevel->getScene()->getMeshInstance(id);

                const SceneID sceneID = instance->getSceneID();
                std::string assetName = mCurrentLevel->getAssetName(sceneID);
                instanceJSon["Asset"] = assetName;
                const float3& position = instance->getPosition();
                const float3& scale = instance->getScale();
                const quat& rotation = instance->getRotation();

                instanceJSon["Position"].insert(0, position.x);
                instanceJSon["Position"].insert(1, position.y);
                instanceJSon["Position"].insert(2, position.z);

                instanceJSon["Rotation"].insert(0, rotation.x);
                instanceJSon["Rotation"].insert(1, rotation.y);
                instanceJSon["Rotation"].insert(2, rotation.z);
                instanceJSon["Rotation"].insert(3, rotation.w);

                instanceJSon["Scale"].insert(0, scale.x);
                instanceJSon["Scale"].insert(1, scale.y);
                instanceJSon["Scale"].insert(2, scale.z);

                instanceJSon["Material"] = mCurrentLevel->getMaterialName(id);

                if(entry.mHasScript)
                {
                    Json::Value script{};
                    script["GamePlay"] = instanceWindow->getScriptNames()[entry.mScriptIndex];

                    instanceJSon["Scripts"] = script;
                }

                if(entry.mHasCollider)
                {
                    Json::Value collider{};
                    collider["Mass"] = entry.mMass;
                    collider["Type"] = entry.mDynamic ? "Dynamic" : "Static";
                    std::string colliderTypes[] = {"Box", "Sphere", "Capsule", "Plane"};
                    collider["Geometry"] = colliderTypes[static_cast<uint32_t>(entry.mCollisionGeom)];

                    instanceJSon["Collider"] = collider;
                }

                instancesJson[name] = instanceJSon;
            }

            root["INSTANCE"] = instancesJson;
        }

        // Export scripts
        {
            const std::vector<std::string>& scripts = instanceWindow->getScriptNames();
            Json::Value scriptsJson{};
            for(const auto& name : scripts)
            {
                scriptsJson[name] = "Scripts/" + name + ".lua";
            }

            root["SCRIPTS"] = scriptsJson;
        }

        // Export Cameras
        {
            const std::unordered_map<std::string, Camera>& cameras = mCurrentLevel->getCameras();
            Json::Value camerasJson{};
            for(const auto&[name, cam] : cameras)
            {
                Json::Value CameraJson{};
                const float3& position = cam.getPosition();
                const float3& direction = cam.getDirection();

                CameraJson["Position"].insert(0, position.x);
                CameraJson["Position"].insert(1, position.y);
                CameraJson["Position"].insert(2, position.z);

                CameraJson["Direction"].insert(0, direction.x);
                CameraJson["Direction"].insert(1, direction.y);
                CameraJson["Direction"].insert(2, direction.z);

                CameraJson["Aspect"] = cam.getAspect();
                CameraJson["NearPlane"] = cam.getNearPlane();
                CameraJson["FarPlane"] = cam.getFarPlane();
                CameraJson["FOV"] = cam.getFOV();
                CameraJson["Mode"] = cam.getMode() == CameraMode::Perspective ? "Perspective" : cam.getMode() == CameraMode::Orthographic ?
                        "Orthographic" : "InfinitePerspective";
                CameraJson["OrthoSize"].insert(0, cam.getOrthographicSize().x);
                CameraJson["OrthoSize"].insert(1, cam.getOrthographicSize().y);

                camerasJson[name] = CameraJson;
            }
            root["CAMERA"] = camerasJson;
        }

        // Export Lights
        {
            Json::Value lightsJson{};

            root["LIGHT"] = lightsJson;
        }

        // Export globals
        {
            Json::Value global_setings{};
            Json::Value Globals{};

            const std::array<std::string, 6>& skybox = mCurrentLevel->getSkybox();
            Json::Value skyboxJson{};
            for(uint32_t i = 0; i< 6; ++i)
                skyboxJson.insert(i, skybox[i]);

            global_setings["Skybox"] = skyboxJson;
            Globals["Global_settings"] = global_setings;

            root["GLOBALS"] = Globals;
        }

        std::ofstream sceneFile{};
        std::filesystem::path sceneFilePath = mCurrentLevel->getWorkingDirectory() / "scene.json";
        sceneFile.open(sceneFilePath);

        sceneFile << root;
    }

    bool SceneWindow::drawInstanceAddWindow()
    {
        bool meshAdded = false;

        if(ImGui::Begin("Add instance"))
        {
            ImGui::InputText("Name:", mInstanceAddTextEdit, 64);

            std::vector<std::string> materials = mCurrentLevel->getMaterials();
            if (ImGui::BeginCombo("Material", mSelectedMaterial.c_str()))
            {
                for (uint32_t n = 0; n < materials.size(); n++)
                {
                    const std::string& matName = materials[n];
                    bool is_selected = matName == mSelectedMaterial;
                    if (ImGui::Selectable(matName.c_str(), is_selected))
                    {
                        mSelectedMaterial = matName;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if(ImGui::Button("Add"))
            {
                mShowInstanceAddWindow = false;
                mCurrentLevel->addMeshInstance(mInstanceAddTextEdit, mAssetToAdd, mSelectedMaterial, float3{0.0f, 0.0f, 0.0f},
                                               quat{1.0f, 0.0f, 0.0f, 0.f}, float3{1.0f, 1.0f, 1.0f});
                std::memset(mInstanceAddTextEdit, 0, 64);
                meshAdded = true;
            }

        }
        ImGui::End();

        return meshAdded;
    }

    void SceneWindow::setLevel(Level* l)
    {
        mCurrentLevel = l;
        std::vector<std::string> materials = mCurrentLevel->getMaterials();
        mSelectedMaterial = materials[0];
    }
}