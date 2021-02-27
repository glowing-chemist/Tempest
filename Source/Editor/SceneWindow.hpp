#ifndef SCENE_WINDOW_HPP
#define SCENE_WINDOW_HPP

#include "Engine/Scene.h"

#include <vector>

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

        void exportSceneToFile();

        const std::vector<InstanceID>& getSelected() const
        {
            return mSelected;
        }

    private:

        std::vector<InstanceID> mSelected;
        Level* mCurrentLevel;

    };
}

#endif