#pragma once

#include "../Utils/Standard.h"


#define DEFINE_EVENT_WITH_DESCRIPTION(eventName) \
    struct eventName##Description;\
    class eventName : public app::Event<app::EventResult(const eventName##Description&)> {};\
    struct eventName##Description


namespace GraphEx
{

struct GRAPHEX_EXPORTABLE EventResult
{
    bool handled = false;
    bool forward = true;
};


template<typename HandlerSignature>
struct Event;


// Meant for subclassing, must define a static bool processResult(ResultT&) function!
template<typename ResultT, typename... HandlerParamTs>
struct Event<ResultT(HandlerParamTs...)>
{
    using Result = ResultT;
    using Handler = std::function<ResultT(HandlerParamTs...)>;
    using HandlerParamsTuple = std::tuple<HandlerParamTs...>;
};


template<typename... HandlerParamTs>
struct Event<EventResult(HandlerParamTs...)>
{
    using Result = EventResult;
    using Handler = std::function<EventResult(HandlerParamTs...)>;
    using HandlerParamsTuple = std::tuple<HandlerParamTs...>;

    static bool processResult(const EventResult& result)
    {
        return result.forward;
    }
};


template<typename EventT>
using EventHandler = typename EventT::Handler;

} // namespace GraphEx::event