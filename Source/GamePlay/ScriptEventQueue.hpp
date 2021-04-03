#ifndef SCRIPT_EVENT_QUEUE_HPP
#define SCRIPT_EVENT_QUEUE_HPP

#include <atomic>
#include <cstdint>
#include <vector>


enum class ScriptEvent
{
    KeyPress = 0,
    KeyHold,
    KeyRelease,
    MouseClick,
    Collision
};

struct Event
{
    ScriptEvent type;
    uint64_t mData1;
    uint64_t mData2;
};

class EventQueue
{
public:
    EventQueue(const uint32_t queueSize);

    bool hasEvents()
    {
        return mWriteIndex != mReadIndex;
    }

    Event readNextEvent();
    void writeEvent(const Event&);

private:

    std::vector<Event> mEvents;
    uint32_t mWriteIndex;
    uint32_t mReadIndex;

};

#endif