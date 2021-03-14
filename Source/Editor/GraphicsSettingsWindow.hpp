#ifndef GRAPHICS_SETTINGS_WINDOW_HPP
#define GRAPHICS_SETTINGS_WINDOW_HPP

class SSAOTechnique;
class CompositeTechnique;

class GraphicsSettingsWindow
{
public:

    GraphicsSettingsWindow() = default;

    void initialize(SSAOTechnique* ssao, CompositeTechnique* comp)
    {
        mSSAO = ssao;
        mComposite = comp;
    }

    void renderUI();

private:
    SSAOTechnique* mSSAO;
    CompositeTechnique* mComposite;

};

#endif