#include "SceneWindow.hpp"
#include "../Level.hpp"

#include "imgui.h"


namespace Tempest
{
    SceneWindow::SceneWindow() :
        mCurrentLevel{nullptr}
    {

    }

    void SceneWindow::renderUI()
    {
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

                if(ImGui::Button("Export Scene"))
                {

                }

                ImGui::End();
            }
        }
    }

    void SceneWindow::exportSceneToFile()
    {

    }



}