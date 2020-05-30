#include "Controller.hpp"
#include <cstdio>
#include <cassert>


Controller::Controller(const int id) : mID(id)
{
    const int present = glfwJoystickPresent(mID);
    assert(present == GLFW_TRUE);

    mName = glfwGetJoystickName(mID);
    printf("Using joystick %s\n", mName);
}


void Controller::update()
{
    mAxis = glfwGetJoystickAxes(mID, &mAxisCount);
    mButtons = glfwGetJoystickButtons(mID, &mButtonCount);
}
