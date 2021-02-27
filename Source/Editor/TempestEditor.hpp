#ifndef TEMPEST_EDITOR_HPP
#define TEMPEST_EDITOR_HPP

#include <filesystem>
#include <memory>

#include "Engine/Camera.hpp"

struct GLFWwindow;
class RenderEngine;

namespace Tempest
{
    class SceneWindow;
    class Level;
    class PhysicsWorld;
    class ScriptEngine;

    class Editor
    {
    public:

        Editor(GLFWwindow* window, std::filesystem::path& directory);

        void run();

    private:

        void pumpInputQueue();
        void initGraphicsState();
        void initImGuiState();

        bool mFirstFrame = true;

        GLFWwindow* mWindow;
        std::filesystem::path mRootDir;

        Level* mCurrentOpenLevel;
        RenderEngine* mRenderEngine;
        PhysicsWorld* mPhysicsEngine;
        ScriptEngine* mScriptEngine;
        SceneWindow* mSceneWindow;

        Camera mEditorCamera;
        double mMousePosition[2];
    };
}

#endif