#include "RenderThread.hpp"

#include "Engine/Engine.hpp"


void run(Tempest::RenderThread* thread)
{
    auto frameStartTime = std::chrono::system_clock::now();

    while(!(thread->mShouldClose))
    {
        std::unique_lock lock(thread->mGraphics_context_mutex);
        thread->mGraphics_cv.wait(lock, [=]{return thread->mReady;});
        thread->mReady = false;

        const auto currentTime = std::chrono::system_clock::now();
        std::chrono::microseconds frameDelta = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - frameStartTime);
        frameStartTime = currentTime;

        if(!(thread->mFirstFrame))
            thread->mEngine->startFrame(frameDelta);

        thread->mEngine->getScene()->computeBounds(AccelerationStructure::Dynamic);

        thread->mEngine->recordScene();
        thread->mEngine->render();
        thread->mEngine->swap();
        thread->mEngine->endFrame();

        thread->mProcessed = true;

        lock.unlock();
        thread->mGraphics_cv.notify_one();
    }

    /*std::unique_lock lock(thread->mGraphics_context_mutex);
    thread->mGraphics_cv.wait(lock, [=]{return thread->mReady;});
    thread->mProcessed = true;
    lock.unlock();
    thread->mGraphics_cv.notify_one();*/
}

namespace Tempest
{

RenderThread::RenderThread(Engine* eng) :
    mEngine(eng)
{
    mThread = std::thread(run, this);
}


RenderThread::~RenderThread()
{
    mThread.join();
}


void RenderThread::update(const bool shouldClose, const bool firstFrame)
{
    mShouldClose = shouldClose;
    mFirstFrame = firstFrame;
}


std::unique_lock<std::mutex> RenderThread::lock()
{
    std::unique_lock lock(mGraphics_context_mutex);
    mGraphics_cv.wait(lock, [this]{return mProcessed;});
    mProcessed = false;

    return lock;
}


void RenderThread::unlock(std::unique_lock<std::mutex>& lock)
{
    mReady = true;
    lock.unlock();
    mGraphics_cv.notify_one();
}

}
