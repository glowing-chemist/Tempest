#ifndef TEMPEST_EDITOR_HPP
#define TEMPEST_EDITOR_HPP

#include <filesystem>
#include <memory>

#include "Engine/Camera.hpp"

struct GLFWwindow;
class RenderEngine;
class InstanceIDTechnique;
class GraphicsSettingsWindow;

namespace Tempest
{
    class SceneWindow;
    class InstanceWindow;
    class Level;
    class PhysicsWorld;
    class ScriptEngine;

    class Editor
    {
    public:

        Editor(GLFWwindow* window, std::filesystem::path& directory);

        void run();

        void text_callback(GLFWwindow* window, unsigned int codePoint);
        void mouseScroll_callback(GLFWwindow*, double, double yoffset);

    private:

        void pumpInputQueue();
        void initGraphicsState();
        void initImGuiState();
        void drawMenuBar();
        void addNewAssets();

        void updateSelectedPhysicsPosition();

        bool mFirstFrame = true;

        GLFWwindow* mWindow;
        std::filesystem::path mRootDir;

        Level* mCurrentOpenLevel;
        RenderEngine* mRenderEngine;
        PhysicsWorld* mPhysicsEngine;
        ScriptEngine* mScriptEngine;
        SceneWindow* mSceneWindow;
        InstanceWindow* mInstanceWindow;

        bool mRenderGraphicsSettingsWindow = false;
        GraphicsSettingsWindow* mGraphicsSettingsWindow;

        InstanceIDTechnique* mInstancePicker;
        uint64_t mSelectedInstance;

        Camera mEditorCamera;
        double mMousePosition[2];
        std::atomic<double> mMouseScrollAmount;
    };
}

#endif