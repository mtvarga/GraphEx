#pragma once

#include "Event.h"
#include "../Utils/DispatchManager.h"


namespace GraphEx
{

using EventId = TypeId;


template<typename EventT>
using EventDispatchManager = DispatchManager<typename EventT::Result, typename EventT::HandlerParamsTuple>;


template<typename EventT>
DispatchTargetInvoker WrapEventHandler(EventHandler<EventT> handler)
{
    return InvokerForDispatchTarget(std::move(handler));
}


struct GRAPHEX_EXPORTABLE EventManager
{
    template<typename EventT>
    void registerEvent();

    template<typename EventT>
    void registerEventHandler(EventHandler<EventT> eventHandler);

    template<typename EventT, typename... HandlerParamTs>
    void dispatchEvent(HandlerParamTs&&... params);

    template<typename EventT, typename... HandlerParamTs>
    void enqueueEvent(HandlerParamTs&&... params);

    void handleEnqueuedEvents() const;

    void cleanup();

    static EventManager& get();

private:
    EventManager() = default;

    std::map<EventId, std::shared_ptr<DispatchManagerBase>> mDispatchManagers;

    static std::unique_ptr<EventManager> pInstance;
};


template<typename EventT, typename = typename EventT::Result>
class MakeEventDispatchManager
{
    std::shared_ptr<EventDispatchManager<EventT>> pValue = std::make_shared<EventDispatchManager<EventT>>(EventT::processResult);

public:
    std::shared_ptr<EventDispatchManager<EventT>> operator()() const
    {
        return pValue;
    }
};


template<typename EventT>
class MakeEventDispatchManager<EventT, void>
{
    std::shared_ptr<EventDispatchManager<EventT>> pValue = std::make_shared<EventDispatchManager<EventT>>();

public:
    std::shared_ptr<EventDispatchManager<EventT>> operator()() const
    {
        return pValue;
    }
};


template<typename EventT>
void EventManager::registerEvent()
{
    if (const EventId eventId = typeid(EventT);
        mDispatchManagers.find(eventId) == mDispatchManagers.end())
    {
        auto makeEventDispatchManager = MakeEventDispatchManager<EventT>();
        mDispatchManagers.emplace(eventId, makeEventDispatchManager());
        return;
    }

    FALCOR_THROW("Attempted to register an Event that has already been registered in EventManager");
}



template<typename EventT>
void EventManager::registerEventHandler(EventHandler<EventT> eventHandler)
{
    const EventId eventId = typeid(EventT);

    if (const auto it = mDispatchManagers.find(eventId);
        it != mDispatchManagers.end())
    {
        it->second->registerTarget(WrapEventHandler<EventT>(std::move(eventHandler)));
        return;
    }

    FALCOR_THROW("Attempted to register an event handler for an Event that has not been registered in EventManager");
}


template<typename EventT, typename... HandlerParamTs>
void EventManager::dispatchEvent(HandlerParamTs&&... params)
{
    const EventId eventId = typeid(EventT);

    if (const auto it = mDispatchManagers.find(eventId);
        it != mDispatchManagers.end())
    {
        const auto& pDispatchManagerBase = it->second;

        if (const auto pDispatchManager = std::static_pointer_cast<EventDispatchManager<EventT>>(pDispatchManagerBase))
        {
            pDispatchManager->performDispatch(std::forward<HandlerParamTs>(params)...);
            return;
        }

        FALCOR_THROW("Failed to dispatch an Event: there was a casting issue when trying to get the DispatchManager for the event");
    }

    FALCOR_THROW("Attempted to dispatch an Event that has not been registered in EventManager");
}


template<typename EventT, typename... HandlerParamTs>
void EventManager::enqueueEvent(HandlerParamTs&&... params)
{
    const EventId eventId = typeid(EventT);

    if (const auto it = mDispatchManagers.find(eventId);
        it != mDispatchManagers.end())
    {
        const auto& pDispatchManagerBase = it->second;

        if (const auto pDispatchManager = std::static_pointer_cast<EventDispatchManager<EventT>>(pDispatchManagerBase))
        {
            pDispatchManager->enqueueDispatch(std::forward<HandlerParamTs>(params)...);
            return;
        }

        FALCOR_THROW("Failed to enqueue an Event: there was a casting issue when trying to get the DispatchManager for the event");
    }

    FALCOR_THROW("Attempted to enqueue an Event that has not been registered in EventManager");
}

}
