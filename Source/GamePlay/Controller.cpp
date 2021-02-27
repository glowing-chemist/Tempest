#include "Controller.hpp"
#include <cstdio>
#include <cassert>
#include <cstring>

#include "Core/Profiling.hpp"

namespace  Tempest {

    Controller::Controller(const int id) : mID(id),
                                           mMouseX(0.0),
                                           mMouseY(0.0) {
        mCtlPressed = false;
        const int present = glfwJoystickPresent(mID);
        if (present == GLFW_TRUE)
            mHardwareController = true;
        else {
            mHardwareController = false;
        }

        mName = glfwGetJoystickName(mID);
        printf("Using joystick %s\n", mName);
    }


    Controller::~Controller() {

    }


    void Controller::update(GLFWwindow *window) {
        PROFILER_EVENT();

        if (mHardwareController) {
            int axisCount;
            const float *axis = glfwGetJoystickAxes(mID, &axisCount);

            int buttonCount;
            const unsigned char *buttons = glfwGetJoystickButtons(mID, &buttonCount);

            std::memcpy(mAxis, axis, sizeof(float) * 6);
            std::memcpy(mButtons, buttons, sizeof(unsigned char) * 8);
        } else {
            for (uint32_t i = 0; i < 6; ++i)
                mAxis[i] = 0.0f;
            for (uint32_t i = 0; i < 8; ++i)
                mButtons[i] = 0;

            if (glfwGetKey(window, mID == GLFW_JOYSTICK_1 ? GLFW_KEY_W : GLFW_KEY_I) == GLFW_PRESS)
                mAxis[1] = -1.0f;
            if (glfwGetKey(window, mID == GLFW_JOYSTICK_1 ? GLFW_KEY_S : GLFW_KEY_K) == GLFW_PRESS)
                mAxis[1] = 1.0f;

            if (glfwGetKey(window, mID == GLFW_JOYSTICK_1 ? GLFW_KEY_A : GLFW_KEY_J) == GLFW_PRESS)
                mAxis[0] = -1.0f;
            if (glfwGetKey(window, mID == GLFW_JOYSTICK_1 ? GLFW_KEY_D : GLFW_KEY_L) == GLFW_PRESS)
                mAxis[0] = 1.0f;

            double x, y;
            glfwGetCursorPos(window, &x, &y);
            mAxis[3] = (x - mMouseX) / 10.0;
            mMouseX = x;
            mAxis[4] = (y - mMouseY) / 10.0;
            mMouseY = y;

            if (glfwGetKey(window, mID == GLFW_JOYSTICK_1 ? GLFW_KEY_SPACE : GLFW_KEY_U) == GLFW_PRESS)
                mButtons[0] = GLFW_PRESS;

            mCtlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
            mShftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        }
    }

}
