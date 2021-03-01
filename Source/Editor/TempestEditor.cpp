#include "TempestEditor.hpp"
#include "Engine/Engine.hpp"
#include "PhysicsWorld.hpp"
#include "ScriptEngine.hpp"
#include "SceneWindow.hpp"
#include "InstanceWindow.hpp"
#include "../Level.hpp"
#include "ImGuizmo.h"

#include "GLFW/glfw3.h"

namespace Tempest
{

    Editor::Editor(GLFWwindow *window, std::filesystem::path &directory) :
        mWindow{window},
        mRootDir(directory),
        mEditorCamera({0.0f, 0.0f, 0.0f},
                      {1.0f, 0.0f, 0.0f},
                      1920.0f / 1080.0f,
                      0.1f,
                      200.0f,
                      90.0f,
                      CameraMode::Perspective)
    {
        glfwSetWindowUserPointer(mWindow, this);

        auto text_callback = [](GLFWwindow* window, unsigned int codePoint)
        {
            Editor* editor = static_cast<Editor*>(glfwGetWindowUserPointer(window));
            editor->text_callback(window, codePoint);
        };
        glfwSetCharCallback(mWindow, text_callback);

        ImGui::CreateContext();

        mRenderEngine = new RenderEngine(mWindow);
        mPhysicsEngine = new PhysicsWorld();
        mScriptEngine = new ScriptEngine();
        mSceneWindow = new SceneWindow(&mEditorCamera);
        mInstanceWindow = new InstanceWindow(mRootDir);

        initGraphicsState();

        mRenderEngine->startFrame(std::chrono::microseconds{0});

        std::filesystem::path sceneFile = mRootDir / "scene.json";
        if(std::filesystem::exists(sceneFile))
        {
            mCurrentOpenLevel = new Level(mRenderEngine, mPhysicsEngine, mScriptEngine, sceneFile, mInstanceWindow, mSceneWindow);
        }
        else
        {
            mCurrentOpenLevel = new Level(mRenderEngine, mPhysicsEngine, mScriptEngine, sceneFile.parent_path(), "NewLevel", mInstanceWindow, mSceneWindow);
        }

        mSceneWindow->setLevel(mCurrentOpenLevel);

        Scene* scene = mCurrentOpenLevel->getScene();
        scene->setCamera(&mEditorCamera);
        mRenderEngine->setScene(scene);

        initImGuiState();
    }

    void Editor::run()
    {
        auto frameStartTime = std::chrono::system_clock::now();

        while(!glfwWindowShouldClose(mWindow))
        {
            ImGui::NewFrame();
            ImGuizmo::BeginFrame();

            const auto currentTime = std::chrono::system_clock::now();
            std::chrono::microseconds frameDelta = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - frameStartTime);
            frameStartTime = currentTime;

            pumpInputQueue();

            if(!mFirstFrame)
                mRenderEngine->startFrame(frameDelta);

            drawMenuBar();
            bool refitNeeded = mSceneWindow->renderUI();
            const std::vector<InstanceID>& selected = mSceneWindow->getSelected();
            for(const auto id : selected)
                refitNeeded = refitNeeded || mInstanceWindow->drawInstanceWindow(mCurrentOpenLevel, id);

            if(refitNeeded)
            {
                mCurrentOpenLevel->getScene()->computeBounds(AccelerationStructure::Dynamic);
                mCurrentOpenLevel->getScene()->computeBounds(AccelerationStructure::Static);
            }

            ImGui::Render();

            mRenderEngine->recordScene();
            mRenderEngine->render();
            mRenderEngine->swap();

            mFirstFrame = false;
        }
    }

    void Editor::pumpInputQueue()
    {
        glfwPollEvents();

        float2 prevMousPosition = {mMousePosition[0], mMousePosition[1]};
        glfwGetCursorPos(mWindow, &mMousePosition[0], &mMousePosition[1]);
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2{static_cast<float>(mMousePosition[0]), static_cast<float>(mMousePosition[1])};
        const float2 cursorDelta = {mMousePosition[0] - prevMousPosition.x, mMousePosition[1] - prevMousPosition.y};

        bool mousePressed[5];
        for(uint32_t i = 0; i < 5; ++i)
        {
            const auto pressed = glfwGetMouseButton(mWindow, i);

            mousePressed[i] = pressed == GLFW_PRESS;
        }

        memcpy(&io.MouseDown[0], &mousePressed[0], sizeof(bool) * 5);

        if(mousePressed[1])
        {
            Camera& currentCam = *mSceneWindow->getCurrentCamera();

            if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
                currentCam.moveForward(0.5f);
            if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
                currentCam.moveBackward(0.5f);
            if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
                currentCam.moveLeft(0.5f);
            if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
                currentCam.moveRight(0.5f);

            currentCam.rotatePitch(cursorDelta.y);
            currentCam.rotateWorldUp(-cursorDelta.x);
        }
    }

    void Editor::text_callback(GLFWwindow*, unsigned int codePoint)
    {
        ImGui::GetIO().AddInputCharacter(codePoint);
    }

    void Editor::initGraphicsState()
    {
        mRenderEngine->registerPass(PassType::DepthPre);
        //mRenderEngine->registerPass(PassType::Shadow);
        mRenderEngine->registerPass(PassType::GBufferPreDepth);
        mRenderEngine->registerPass(PassType::DeferredPBRIBL);
        mRenderEngine->registerPass(PassType::DFGGeneration);
        mRenderEngine->registerPass(PassType::Skybox);
        mRenderEngine->registerPass(PassType::ConvolveSkybox);
        mRenderEngine->registerPass(PassType::Composite);
        mRenderEngine->registerPass(PassType::Animation);
        mRenderEngine->registerPass(PassType::LineariseDepth);
        mRenderEngine->registerPass(PassType::LightFroxelation);
        mRenderEngine->registerPass(PassType::DeferredAnalyticalLighting);
        mRenderEngine->registerPass(PassType::DebugAABB);
        mRenderEngine->registerPass(PassType::Overlay);
    }


    void Editor::initImGuiState()
    {
        ImGuiIO& io = ImGui::GetIO();

        int display_w, display_h;
        glfwGetFramebufferSize(mWindow, &display_w, &display_h);
        io.DisplaySize = ImVec2(static_cast<float>(display_w), static_cast<float>(display_h));
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::SetOrthographic(false);

        {
            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        }
    }

    void Editor::drawMenuBar()
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Export"))
            {
                mSceneWindow->exportSceneToFile(mInstanceWindow);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
