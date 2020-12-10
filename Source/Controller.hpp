#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <GLFW/glfw3.h>

class Controller
{
public:

    Controller(const int joyStickID);
    ~Controller();


    void update(GLFWwindow *window);

    float getLeftAxisX() const
    {
        return mAxis[0];
    }
    float getLeftAxisY() const
    {
        return mAxis[1];
    }

    float getRighAxisX() const
    {
        return mAxis[3];
    }
    float getRighAxisY() const
    {
        return mAxis[4];
    }

    bool pressedX() const
    {
        return mButtons[0] == GLFW_PRESS;
    }

    bool releasedX() const
    {
        return mButtons[0] == GLFW_RELEASE;
    }

private:

    const char* mName;
    int mID;

    float mAxis[6];

    unsigned char mButtons[8];

    double mMouseX, mMouseY;
    bool mHardwareController;
};


#endif
