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
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Tempest::TempestEngine* engine = new Tempest::TempestEngine(window, ".");

    if(argc > 1)
        engine->loadLevel(argv[1]);
    else
        engine->loadLevel("./Assets/SandBox.json");

    engine->run();
}
