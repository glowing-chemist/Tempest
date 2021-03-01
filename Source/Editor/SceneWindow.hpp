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
        SceneWindow(Camera* editorCam);

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

        Camera* getCurrentCamera()
        {
            return mCurrentCamera;
        }

    private:

        bool drawInstanceAddWindow();
        void drawCameraAddWindow();

        // Data for add instance window.
        char mInstanceAddTextEdit[64];
        SceneID mAssetToAdd;
        std::string mSelectedMaterial;
        bool mShowInstanceAddWindow = false;

        // Data for add camera Window
        char mCameraAddTextEdit[64];
        std::string mCurrentCameraName;
        Camera* mCurrentCamera;
        std::vector<std::pair<std::string, Camera*>> mCameras;
        bool mShowAddCameraWindow = false;

        std::vector<InstanceID> mSelected;
        std::unordered_map<SceneID, bool> mAssetDynamism;
        Level* mCurrentLevel;

    };
}

#endif