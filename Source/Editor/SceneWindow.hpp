#ifndef SCENE_WINDOW_HPP
#define SCENE_WINDOW_HPP

#include "Engine/Scene.h"

#include <vector>

namespace Tempest
{
    class Level;
    class InstanceWindow;

    class SceneWindow
    {
    public:
        SceneWindow();

        void setLevel(Level* l);

        bool renderUI();

        void exportSceneToFile(const InstanceWindow*);

        const std::vector<InstanceID>& getSelected() const
        {
            return mSelected;
        }

        void setAssetDynamic(const SceneID id, const bool d)
        {
            mAssetDynamism[id] = d;
        }

    private:

        bool drawInstanceAddWindow();

        // Data for add instance window.
        char mInstanceAddTextEdit[64];
        SceneID mAssetToAdd;
        std::string mSelectedMaterial;
        bool mShowInstanceAddWindow = false;

        std::vector<InstanceID> mSelected;
        std::unordered_map<SceneID, bool> mAssetDynamism;
        Level* mCurrentLevel;

    };
}

#endif