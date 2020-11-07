#ifndef RENDERTHREAD_HPP
#define RENDERTHREAD_HPP

#include <condition_variable>
#include <mutex>
#include <thread>

class Engine;

namespace Tempest
{

class RenderThread
{
public:
    RenderThread(Engine*);
    ~RenderThread();

    RenderThread(const RenderThread&) = delete;
    RenderThread(RenderThread&&) = delete;

    RenderThread& operator=(const RenderThread&) = delete;
    RenderThread& operator=(RenderThread&&) = delete;


    void update(const bool shouldClose, const bool firstFrame);
    // lock acces to internal render thread data.
    std::unique_lock<std::mutex> lock();
    // Kick render thread.
    void unlock(std::unique_lock<std::mutex>&);

    Engine* mEngine;

    std::thread mThread;
    bool mProcessed = true;
    bool mReady = false;
    bool mShouldClose = false;
    bool mFirstFrame = true;
    std::mutex mGraphics_context_mutex;
    std::condition_variable mGraphics_cv;
};

}

#endif
