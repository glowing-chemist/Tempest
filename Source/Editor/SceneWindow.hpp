#ifndef SCENE_WINDOW_HPP
#define SCENE_WINDOW_HPP

namespace Tempest
{
    class Level;

    class SceneWindow
    {
    public:
        SceneWindow();

        void setLevel(Level* l)
        {
            mCurrentLevel = l;
        }

        void renderUI();

    private:

        Level* mCurrentLevel;

    };
}

#endif