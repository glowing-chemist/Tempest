#include "TempestEngine.hpp"
#include "RenderThread.hpp"
#include "ScriptEngine.hpp"
#include "PhysicsWorld.hpp"
#include "Level.hpp"

#include "Engine/Engine.hpp"

namespace Tempest
{
    TempestEngine::TempestEngine(GLFWwindow *window, const std::filesystem::path& path) :
        mWindow(window),
        mCurrentLevel{nullptr},
        mRootDir(path),
        mScene(nullptr)
    {
        mRenderEngine = new RenderEngine(mWindow);
        mRenderThread = nullptr;
        mPhysicsEngine = new PhysicsWorld();
        mScriptEngine = new ScriptEngine();

        mRenderEngine->startFrame(std::chrono::microseconds(0));
    }


    TempestEngine::~TempestEngine()
    {
        delete mRenderThread;
        delete mRenderEngine;
        delete mPhysicsEngine;
        delete mScriptEngine;
    }


    void TempestEngine::loadLevel(const std::string& path)
    {
        delete mCurrentLevel;
        mCurrentLevel = new Level(mRenderEngine, mPhysicsEngine, mScriptEngine, path, "test level");

        mRenderEngine->setScene(mCurrentLevel->getScene());
    }


    void TempestEngine::run()
    {
        setupGraphicsState();

        mRenderEngine->setShadowMapResolution({1024.0f, 1024.0f});

        mRenderThread  = new RenderThread(mRenderEngine);
        auto frameStartTime = std::chrono::system_clock::now();

        while (!mShouldClose)
        {
            PROFILER_START_FRAME("Start frame");

            glfwPollEvents();

            mShouldClose = glfwWindowShouldClose(mWindow);

            const auto currentTime = std::chrono::system_clock::now();
            std::chrono::microseconds frameDelta = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - frameStartTime);
            frameStartTime = currentTime;

            mPhysicsEngine->tick(frameDelta);

            {
                mRenderThread->update(mShouldClose, mFirstFrame);
                std::unique_lock lock = mRenderThread->lock();

                mScriptEngine->tick(frameDelta);

                mRenderThread->unlock(lock);
            }

            mFirstFrame = false;
        }
    }

    void TempestEngine::translateInstance(const InstanceID id, const float3& v)
    {
        mScene->translateInstance(id, v);
        mPhysicsEngine->translateInstance(id, v);
    }

    float3 TempestEngine::getInstancePosition(const InstanceID id) const
    {
        return mScene->getInstancePosition(id);
    }


    void   TempestEngine::setInstancePosition(const InstanceID id, const float3& v)
    {
        mScene->setInstancePosition(id, v);
        mPhysicsEngine->setInstancePosition(id, v);
    }

    void TempestEngine::startAnimation(const InstanceID id, const std::string& name, const bool loop, const float speedModifer)
    {
        mRenderEngine->startAnimation(id, name, loop, speedModifer);
    }


    void TempestEngine::terimateAnimation(const InstanceID id, const std::string& name)
    {
        mRenderEngine->terimateAnimation(id, name);
    }

    InstanceID TempestEngine::getInstanceIDByName(const std::string& name) const
    {
        BELL_ASSERT(mCurrentLevel, "No level loaded")
        return mCurrentLevel->getInstanceIDByName(name);
    }

    SceneID TempestEngine::getSceneIDByName(const std::string& name) const
    {
        BELL_ASSERT(mCurrentLevel, "No level loaded")
        return mCurrentLevel->getSceneIDByname(name);
    }

    void TempestEngine::setMainCameraByName(const std::string& name)
    {
        BELL_ASSERT(mCurrentLevel, "No level loaded")
        mCurrentLevel->setMainCameraByName(name);
    }

    void TempestEngine::setShadowCameraByName(const std::string& name)
    {
        BELL_ASSERT(mCurrentLevel, "No level loaded")
        mCurrentLevel->setShadowCameraByName(name);
    }

    void TempestEngine::setupGraphicsState()
    {
        mRenderEngine->registerPass(PassType::DepthPre);
        mRenderEngine->registerPass(PassType::Shadow);
        mRenderEngine->registerPass(PassType::GBufferPreDepth);
        mRenderEngine->registerPass(PassType::DeferredPBRIBL);
        mRenderEngine->registerPass(PassType::DFGGeneration);
        mRenderEngine->registerPass(PassType::Skybox);
        mRenderEngine->registerPass(PassType::ConvolveSkybox);
        mRenderEngine->registerPass(PassType::Composite);
        mRenderEngine->registerPass(PassType::Animation);
        mRenderEngine->registerPass(PassType::LineariseDepth);
        //eng->registerPass(PassType::TAA);
        //eng->registerPass(PassType::SSAO);
#ifndef NDEBUG
        mRenderEngine->registerPass(PassType::DebugAABB);
#endif
    }
}
