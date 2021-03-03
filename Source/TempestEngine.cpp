#include "TempestEngine.hpp"
#include "RenderThread.hpp"
#include "ScriptEngine.hpp"
#include "PhysicsWorld.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "Controller.hpp"

#include "Engine/Engine.hpp"

namespace Tempest
{
    TempestEngine::TempestEngine(GLFWwindow *window, const std::filesystem::path& path) :
        mWindow(window),
        mCurrentLevel{nullptr},
        mRootDir(path)
    {
        mRenderEngine = new RenderEngine(mWindow);
        mRenderThread = nullptr;
        mPhysicsEngine = new PhysicsWorld();
        mScriptEngine = new ScriptEngine();

        mScriptEngine->registerEngineHooks(this);
        mScriptEngine->registerPhysicsHooks(mPhysicsEngine);

        mRenderEngine->startFrame(std::chrono::microseconds(0));
    }


    TempestEngine::~TempestEngine()
    {
        delete mRenderThread;
        delete mRenderEngine;
        delete mPhysicsEngine;
        delete mScriptEngine;
    }


    void TempestEngine::loadLevel(const std::filesystem::path& path)
    {
        delete mCurrentLevel;
        mCurrentLevel = new Level(mRenderEngine, mPhysicsEngine, mScriptEngine, mRootDir / path);

        mRenderEngine->setScene(mCurrentLevel->getScene());
        mScriptEngine->registerSceneHooks(mCurrentLevel->getScene());

        mScriptEngine->init();
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

                mPhysicsEngine->updateDynamicObjects(mCurrentLevel->getScene());

                mScriptEngine->tick(frameDelta);

                mRenderThread->unlock(lock);
            }

            mFirstFrame = false;
        }
    }

    void TempestEngine::startInstanceFrame(const InstanceID id)
    {
        Instance* inst = mCurrentLevel->getScene()->getMeshInstance(id);
        inst->newFrame();
    }

    void TempestEngine::setInstanceLinearVelocity(const InstanceID id, const float3& v)
    {
        mPhysicsEngine->setInstanceLinearVelocity(id, v);
    }

    void TempestEngine::translateInstance(const InstanceID id, const float3& v)
    {
        mCurrentLevel->getScene()->translateInstance(id, v);
        mPhysicsEngine->translateInstance(id, v);
    }

    float3 TempestEngine::getInstancePosition(const InstanceID id) const
    {
        return mCurrentLevel->getScene()->getInstancePosition(id);
    }


    void   TempestEngine::setInstancePosition(const InstanceID id, const float3& v)
    {
        mCurrentLevel->getScene()->setInstancePosition(id, v);
        mPhysicsEngine->setInstancePosition(id, v);
    }

    void   TempestEngine::setInstanceRotation(const InstanceID id, const quat& rot)
    {
        mCurrentLevel->getScene()->getMeshInstance(id)->setRotation(rot);
        mPhysicsEngine->setInstanceRotation(id, rot);
    }

    void   TempestEngine::setGraphicsInstancePosition(const InstanceID id, const float3& v)
    {
        mCurrentLevel->getScene()->setInstancePosition(id, v);
    }

    float3 TempestEngine::getInstanceSize(const InstanceID id) const
    {
        const MeshInstance* meshInstance = mCurrentLevel->getScene()->getMeshInstance(id);

        AABB transformedAABB = meshInstance->getMesh()->getAABB() * meshInstance->getTransMatrix();
        return transformedAABB.getSideLengths();
    }

    float3 TempestEngine::getInstanceCenter(const InstanceID id) const
    {
        const MeshInstance* meshInstance = mCurrentLevel->getScene()->getMeshInstance(id);

        AABB transformedAABB = meshInstance->getMesh()->getAABB() * meshInstance->getTransMatrix();
        return transformedAABB.getCentralPoint();
    }

    float3 TempestEngine::getPhysicsBodyPosition(const InstanceID id)
    {
        btRigidBody* body = mPhysicsEngine->getRigidBody(id);
        const btVector3& center = body->getCenterOfMassPosition();

        return {center.x(), center.y(), center.z()};
    }

    void TempestEngine::updatePlayersAttachedCameras(const InstanceID id)
    {
        auto& p = mPlayers[id];
        auto& camera = mControllers[id];
        p->updateCameras(camera.get());
    }

    void TempestEngine::startAnimation(const InstanceID id, const std::string& name, const bool loop, const float speedModifer)
    {
        mRenderEngine->startAnimation(id, name, loop, speedModifer);
    }


    void TempestEngine::terminateAnimation(const InstanceID id, const std::string& name)
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

    void TempestEngine::createPlayerInstance(const InstanceID id, const float3& pos, const float3& dir)
    {
        auto p = std::make_unique<Player>(id, mCurrentLevel->getScene(), pos, dir);
        mPlayers[id] = std::move(p);
    }

    const Controller& TempestEngine::getControllerForInstance(const InstanceID id)
    {
        BELL_ASSERT(mControllers.find(id) != mControllers.end(), "No controller created for this instance")
        auto& controller = mControllers[id];

        return *controller.get();
    }

    void TempestEngine::createControllerInstance(const InstanceID id, const uint32_t joyStickIndex)
    {
        auto c = std::make_unique<Controller>(joyStickIndex);
        mControllers[id] = std::move(c);
    }

    const Controller& TempestEngine::updateControllerInstance(const InstanceID id)
    {
        BELL_ASSERT(mControllers.find(id) != mControllers.end(), "No controller created for this instance")
        auto& controller = mControllers[id];

        controller->update(mWindow);

        return *controller.get();
    }

    void TempestEngine::attachCameraToPlayer(const InstanceID id, const std::string& n, const float armatureLenght)
    {
        BELL_ASSERT(mPlayers.find(id) != mPlayers.end(), "No player created for this instance")
        auto& p = mPlayers[id];
        Camera& cam = mCurrentLevel->getCameraByName(n);

        p->attachCamera(cam, armatureLenght);
    }

    void TempestEngine::attachShadowCameraToPlayer(const InstanceID id, const std::string& n)
    {
        BELL_ASSERT(mPlayers.find(id) != mPlayers.end(), "No player created for this instance")
        auto& p = mPlayers[id];
        Camera& cam = mCurrentLevel->getCameraByName(n);

        p->attachShadowCamera(cam);
    }

    void TempestEngine::applyImpulseToInstance(const InstanceID id, const float3& impulse)
    {
        btRigidBody* body = mPhysicsEngine->getRigidBody(id);
        if (!body->isActive())
            body->activate(true);
        body->applyCentralImpulse({impulse.x, impulse.y, impulse.z});
    }

    float3 TempestEngine::getCameraDirectionByName(const std::string& n) const
    {
        const Camera& cam = mCurrentLevel->getCameraByName(n);
        return cam.getDirection();
    }

    float3 TempestEngine::getCameraRightByName(const std::string& n) const
    {
        const Camera& cam = mCurrentLevel->getCameraByName(n);
        return cam.getRight();
    }

    float3 TempestEngine::getCameraPositionByName(const std::string& n) const
    {
        const Camera& cam = mCurrentLevel->getCameraByName(n);
        return cam.getPosition();
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
        mRenderEngine->registerPass(PassType::LightFroxelation);
        mRenderEngine->registerPass(PassType::DeferredAnalyticalLighting);
        //eng->registerPass(PassType::TAA);
        //eng->registerPass(PassType::SSAO);
#ifndef NDEBUG
        mRenderEngine->registerPass(PassType::DebugAABB);
#endif
    }
}
