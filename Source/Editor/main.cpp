#include "TempestEditor.hpp"

#include "GLFW/glfw3.h"


int main(int argc, char **argv)
{
    std::filesystem::path workingDirectoy;
    if(argc != 2) {
        printf("Need to supply the working directory on the command line\n");
        return 0;
    }
    else
        workingDirectoy = argv[1];

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // only resize explicitly

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    const uint32_t windowWidth = mode->width;
    const uint32_t windowHeight = mode->height;
    auto* window = glfwCreateWindow(windowWidth, windowHeight, "TempestEditor", nullptr, nullptr);

    auto* editor = new Tempest::Editor(window, workingDirectoy);

    editor->run();

    delete editor;
}