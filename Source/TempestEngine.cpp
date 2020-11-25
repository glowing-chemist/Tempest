#include "TempestEngine.hpp"
#include "RenderThread.hpp"

#include "Engine/Engine.hpp"

namespace Tempest
{
    TempestEngine::TempestEngine(GLFWwindow *window, const std::filesystem::path& path) :
        mRootDir(path)
    {
        mEngine = new Engine(window);
        mRenderThread = new RenderThread(mEngine);
    }
}
