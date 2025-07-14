#include "EventManager.h"

using namespace GraphEx;


std::unique_ptr<EventManager> EventManager::pInstance(new EventManager);


void EventManager::handleEnqueuedEvents() const
{
    for (const auto& [eventId, pDispatchManager] : mDispatchManagers)
    {
        pDispatchManager->handleEnqueuedDispatches();
    }
}


void EventManager::cleanup()
{
    mDispatchManagers.clear();
}


EventManager& EventManager::get()
{
    static EventManager instance;
    return instance;
}