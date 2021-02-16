#ifndef TEMPEST_ENGINE_HPP
#define TEMPEST_ENGINE_HPP

#include <filesystem>

class RenderEngine;
struct GLFWwindow;

namespace Tempest
{

class RenderThread;

class TempestEngine
{
public:
    TempestEngine(GLFWwindow* window, const std::filesystem::path& rootDir);
    ~TempestEngine();

    void run();

private:

    std::filesystem::path mRootDir;
    RenderEngine* mEngine;
    RenderThread* mRenderThread;

};

}

#endif
