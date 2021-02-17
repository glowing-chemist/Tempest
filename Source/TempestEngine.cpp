#include "TempestEngine.hpp"
#include "RenderThread.hpp"
#include "ScriptEngine.hpp"

#include "Engine/Engine.hpp"

namespace Tempest
{
    TempestEngine::TempestEngine(GLFWwindow *window, const std::filesystem::path& path) :
        mRootDir(path)
    {
        mRenderEngine = new RenderEngine(window);
        mRenderThread = new RenderThread(mRenderEngine);
        mScriptEngine = new ScriptEngine();
    }


    TempestEngine::~TempestEngine()
    {
        delete mRenderThread;
        delete mRenderEngine;
        delete mScriptEngine;
    }


    void TempestEngine::run()
    {

    }

}
