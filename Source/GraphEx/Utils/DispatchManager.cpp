#include "DispatchManager.h"


using namespace GraphEx;


DispatchTargetInvoker GraphEx::InvokerForDispatchTarget(const std::function<void()>& target)
{
    return [target](void*, const void*) { target(); };
}


void DispatchManagerBase::registerTarget(DispatchTargetInvoker targetInvoker)
{
    mTargetInvokers.emplace_back(move(targetInvoker));
}


void DispatchManager<void, std::tuple<>>::handleEnqueuedDispatches()
{
    if (mEnqueued)
    {
        performDispatch();
        mEnqueued = false;
    }
}


void DispatchManager<void, std::tuple<>>::performDispatch() const
{
    for (const auto& callDispatchHandler : mTargetInvokers)
    {
        callDispatchHandler(nullptr, nullptr);
    }
}


void DispatchManager<void, std::tuple<>>::enqueueDispatch()
{
    mEnqueued = true;
}
