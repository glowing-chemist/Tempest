#include "InstanceWindow.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "glm/gtc/type_ptr.hpp"
#include "../Level.hpp"

namespace Tempest
{
    InstanceWindow::InstanceWindow(const std::filesystem::path& dir) :
            mWorkingDir{dir}
    {
        // Find all script files.
        std::filesystem::path scriptDir = mWorkingDir / "Scripts";
        for(const auto child : std::filesystem::directory_iterator(scriptDir))
        {
            if(std::filesystem::is_regular_file(child))
            {
                if(child.path().extension().string() == ".lua")
                {
                    mScriptNames.push_back(child.path().stem().string());
                }
            }
        }
    }

    bool InstanceWindow::drawInstanceWindow(Level* level, const InstanceID id)
    {
        Scene* scene = level->getScene();
        MeshInstance* instance = scene->getMeshInstance(id);
        std::string name = instance->getName();
        bool modified = false;
        if(ImGui::Begin(instance->getName().c_str()))
        {
            const Camera& camera = scene->getCamera();
            const float4x4 view = glm::lookAt(camera.getPosition(), camera.getPosition() - camera.getDirection(), float3(0.0f, -1.0f, 0.0f));
            const float4x4 proj = camera.getProjectionMatrix();

            InstanceEntry& entry = mInstanceInfo[id];

            instance->newFrame();

            float3 position = instance->getPosition();
            modified = ImGui::InputFloat3("Position", &position.x);

            float3 scale = instance->getScale();
            modified = modified || ImGui::InputFloat3("Scale", &scale.x);

            quat rotation = instance->getRotation();
            modified = modified || ImGui::InputFloat4("Rotation", &rotation.x);

            {
                ImGui::BeginGroup();

                ImGui::RadioButton("Translate", &entry.mGuizmoIndex, 0);
                ImGui::RadioButton("Rotate", &entry.mGuizmoIndex, 1);
                ImGui::RadioButton("Scale", &entry.mGuizmoIndex, 2);

                ImGui::EndGroup();
            }

            if(!modified) {
                float4x4 mat = instance->getTransMatrix();
                modified = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), static_cast<ImGuizmo::OPERATION>(entry.mGuizmoIndex),
                                     entry.mGuizmoIndex == 0 ? ImGuizmo::MODE::WORLD : ImGuizmo::MODE::LOCAL,
                                     glm::value_ptr(mat));

                float3 skew;
                float4 persp;
                glm::decompose(mat, scale, rotation, position, skew, persp);
            }

            instance->setPosition(position);
            instance->setScale(scale);
            instance->setRotation(rotation);

            // Calculate the size of teh scaled mesh in meters.
            {
                AABB aabb = instance->getMesh()->getAABB();
                const float4x4 transform = instance->getTransMatrix();
                aabb = aabb * transform;
                const float3 size = aabb.getSideLengths();
                ImGui::Text("Size X %fm Y %fm Z %fm", size.x, size.y, size.z);
            }

            std::vector<std::string> materials = level->getMaterials();
            uint32_t subMeshCount = instance->getSubMeshCount();
            for(uint32_t i = 0; i < subMeshCount; ++i)
            {
                ImGui::PushID(i);
                std::string activeMaterial = level->getMaterialName(id, i);
                if (ImGui::BeginCombo("Material", activeMaterial.c_str()))
                {
                    for (uint32_t n = 0; n < materials.size(); n++)
                    {
                        const std::string& matName = materials[n];
                        bool is_selected = matName == activeMaterial;
                        if (ImGui::Selectable(matName.c_str(), is_selected))
                        {
                            activeMaterial = matName;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();
                level->setInstanceMaterial(id, i, activeMaterial);
            }

            ImGui::Checkbox("Has Script", &entry.mHasScript);
            if(entry.mHasScript)
            {
                std::string activeScript = mScriptNames[entry.mScriptIndex];
                if (ImGui::BeginCombo("Active script", activeScript.c_str()))
                {
                    for (uint32_t n = 0; n < mScriptNames.size(); n++)
                    {
                        const std::string& scriptName = mScriptNames[n];
                        bool is_selected = scriptName == activeScript;
                        if (ImGui::Selectable(scriptName.c_str(), is_selected))
                        {
                            entry.mScriptIndex = n;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::Checkbox("Has collider", &entry.mHasCollider);
            if(entry.mHasCollider)
            {
                ImGui::InputFloat("Mass", &entry.mMass);
                ImGui::InputFloat("Restitution", &entry.mRestitution);

                std::string physicsTypes[] = {"Dynamic", "Static", "Kinematic"};
                const std::string& activeEntityType = physicsTypes[static_cast<uint32_t>(entry.mColliderType)];
                if (ImGui::BeginCombo("Entity type", activeEntityType.c_str()))
                {
                    for (uint32_t n = 0; n < 3; n++)
                    {
                        const std::string& typeName = physicsTypes[n];
                        bool is_selected = typeName == activeEntityType;
                        if (ImGui::Selectable(typeName.c_str(), is_selected))
                        {
                            entry.mColliderType = static_cast<PhysicsEntityType>(n);
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                std::string colliderTypes[] = {"Box", "Sphere", "Capsule", "Plane"};
                const std::string& activeCollider = colliderTypes[static_cast<uint32_t>(entry.mCollisionGeom)];
                if (ImGui::BeginCombo("Collider", activeCollider.c_str()))
                {
                    for (uint32_t n = 0; n < 4; n++)
                    {
                        const std::string& colliderName = colliderTypes[n];
                        bool is_selected = colliderName == activeCollider;
                        if (ImGui::Selectable(colliderName.c_str(), is_selected))
                        {
                            entry.mCollisionGeom = static_cast<BasicCollisionGeometry>(n);
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            if(ImGui::Button("Delete"))
            {
                level->removeInstanceByName(name);
                mInstanceInfo.erase(id);
                modified = true;
            }
        }
        ImGui::End();

        return modified;
    }

    void InstanceWindow::setInstanceScript(const InstanceID id, const std::string& script)
    {
        InstanceEntry& entry = mInstanceInfo[id];
        entry.mHasScript = true;

        auto scriptIt = std::find(mScriptNames.begin(), mScriptNames.end(), script);
        const uint32_t index = std::distance(mScriptNames.begin(), scriptIt);
        entry.mScriptIndex = index;
    }


    void InstanceWindow::setInstanceCollider(const InstanceID id,
                                             const BasicCollisionGeometry geom,
                                             const float mass,
                                             const PhysicsEntityType type,
                                             const float restitution)
    {
        InstanceEntry& entry = mInstanceInfo[id];
        entry.mHasCollider = true;
        entry.mCollisionGeom = geom;
        entry.mColliderType = type;
        entry.mMass = mass;
        entry.mRestitution = restitution;
    }

}
