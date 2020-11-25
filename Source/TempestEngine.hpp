#ifndef TEMPEST_ENGINE_HPP
#define TEMPEST_ENGINE_HPP

#include <filesystem>

class Engine;
class GLFWwindow;

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
    Engine* mEngine;
    RenderThread* mRenderThread;

};

}

#endif
