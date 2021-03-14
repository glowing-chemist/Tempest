#include "GraphicsSettingsWindow.hpp"
#include "Engine/SSAOTechnique.hpp"
#include "Engine/CompositeTechnique.hpp"
#include "imgui.h"


void GraphicsSettingsWindow::renderUI()
{
    if(ImGui::Begin("Graphics Settings"))
    {

        if (mSSAO)
        {
            if(ImGui::TreeNode("SSAO"))
            {
                float &radius = mSSAO->getRadius();
                float &bias = mSSAO->getBias();
                float &intensity = mSSAO->getIntensity();

                ImGui::DragFloat("Radius", &radius, 0.1f, 0.1f, 5.0f);
                ImGui::DragFloat("bias", &bias, 0.01f, 0.001f, 1.0f);
                ImGui::DragFloat("intensity", &intensity, 0.2f, 0.5f, 10.0f);

                ImGui::TreePop();
            }
        }

        if(mComposite)
        {
            if(ImGui::TreeNode("Composite"))
            {
                float& gamma = mComposite->getGamma();
                float& exposure = mComposite->getExposure();

                ImGui::DragFloat("Gamma", &gamma, 0.1f, 0.8f, 3.0f);
                ImGui::DragFloat("exposure", &exposure, 0.1f, 0.1f, 2.0f);

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}