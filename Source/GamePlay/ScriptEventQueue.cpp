#include "ScriptEventQueue.hpp"


EventQueue::EventQueue(const uint32_t queueSize) :
    mWriteIndex(0),
    mReadIndex(0)
{
    mEvents.resize(queueSize);
}

Event EventQueue::readNextEvent()
{
    Event e = mEvents[mReadIndex];
    mReadIndex = mReadIndex + 1 & mEvents.size();
    return e;
}


void EventQueue::writeEvent(const Event& event)
{
    mEvents[mWriteIndex] = event;
    mWriteIndex = mWriteIndex + 1 % mEvents.size();
}