#pragma once

#include "../API/Event.h"
#include "CoreTypes.h"


namespace GraphEx::Core
{

struct GRAPHEX_EXPORTABLE EventFrameWillBegin : Event<void()> {};
struct GRAPHEX_EXPORTABLE EventRenderWillBegin : Event<void()> {};
struct GRAPHEX_EXPORTABLE EventRenderBegan : Event<void()> {};
struct GRAPHEX_EXPORTABLE EventRenderWillEnd : Event<void()> {};
struct GRAPHEX_EXPORTABLE EventRenderEnded : Event<void()> {};
struct GRAPHEX_EXPORTABLE EventFrameEnded : Event<void()> {};


struct GRAPHEX_EXPORTABLE KeyboardEvent : Event<bool(const Falcor::KeyboardEvent&)>
{
    static bool handled;

    static bool processResult(const bool& result)
    {
        handled = result;
        return !result;
    }
};


struct GRAPHEX_EXPORTABLE MouseEvent : Event<bool(const Falcor::MouseEvent&)>
{
    static bool handled;

    static bool processResult(const bool& result)
    {
        handled = result;
        return !result;
    }
};


struct GRAPHEX_EXPORTABLE GamepadEvent : Event<bool(const Falcor::GamepadState&)>
{
    static bool handled;

    static bool processResult(const bool& result)
    {
        handled = result;
        return !result;
    }
};


struct GRAPHEX_EXPORTABLE EventSceneObjectAdded : Event<void(std::shared_ptr<SceneObject>)> {};
struct GRAPHEX_EXPORTABLE EventSceneObjectRemoved : Event<void(std::shared_ptr<SceneObject>)> {};


} // namespace GraphEx::Core
