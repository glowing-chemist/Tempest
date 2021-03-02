#include "Engine/Engine.hpp"
#include "Engine/StaticMesh.h"

#include "GLFW/glfw3.h"

#include <glm/gtx/transform.hpp>

#include "TempestEngine.hpp"




int main(int argc, char **argv)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // only resize explicitly
    auto* window = glfwCreateWindow(1920, 1080, "Tempest", nullptr, nullptr);

    if(argc == 2)
    {
        Tempest::TempestEngine *engine = new Tempest::TempestEngine(window, argv[1]);

        engine->loadLevel("scene.json");

        engine->run();
    }
    else
    {
        printf("Working directory required argument");
    }
}
