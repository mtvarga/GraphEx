#pragma once

#include "Standard.h"


namespace GraphEx
{

using DispatchTargetInvoker = std::function<void(void* pResult, const void* pParamsTuple)>;


struct GRAPHEX_EXPORTABLE DispatchManagerBase
{
    virtual ~DispatchManagerBase() = default;

    virtual void handleEnqueuedDispatches() = 0;
    void registerTarget(DispatchTargetInvoker targetInvoker);

protected:
    std::vector<DispatchTargetInvoker> mTargetInvokers;
};


template<typename ResultT, typename... TargetParamTs>
DispatchTargetInvoker InvokerForDispatchTarget(std::function<ResultT(TargetParamTs...)> target)
{
    return [target](void* pResult, const void* pParamsTuple)
    {
        using TargetParamsTuple = std::tuple<TargetParamTs...>;

        auto pResultTyped = static_cast<ResultT*>(pResult);
        const auto& pParamsTupleTyped = static_cast<const TargetParamsTuple*>(pParamsTuple);

        *pResultTyped = std::apply(target, *pParamsTupleTyped);
    };
}


template<typename ResultT>
DispatchTargetInvoker InvokerForDispatchTarget(std::function<ResultT()> target)
{
    return [target](void* pResult, const void*)
    {
        auto pResultTyped = static_cast<ResultT*>(pResult);
        *pResultTyped = target();
    };
}


template<typename... TargetParamTs>
DispatchTargetInvoker InvokerForDispatchTarget(std::function<void(TargetParamTs...)> target)
{
    return [target](void*, const void* pParamsTuple)
    {
        using TargetParamsTuple = std::tuple<TargetParamTs...>;

        auto pParamsTupleTyped = static_cast<const TargetParamsTuple*>(pParamsTuple);
        std::apply(target, *pParamsTupleTyped);
    };
}


GRAPHEX_EXPORTABLE DispatchTargetInvoker InvokerForDispatchTarget(const std::function<void()>& target);


template<typename ResultT, typename TargetParamsTupleT>
struct DispatchManager final : DispatchManagerBase
{
    using ResultCallback = std::function<bool(ResultT&)>;

    explicit DispatchManager(ResultCallback dispatchResultCallback = [](ResultT&) { return true; })
        : mDispatchResultCallback(std::move(dispatchResultCallback)) {}

    void handleEnqueuedDispatches() override;

    template<typename... TargetParamTs>
    void performDispatch(TargetParamTs&&... args);

    template<typename... TargetParamTs>
    void enqueueDispatch(TargetParamTs&&... args);

private:
    void dispatch(const TargetParamsTupleT& paramsTuple);

    std::queue<TargetParamsTupleT> mDispatches;
    ResultCallback mDispatchResultCallback;

public:
    DEFAULT_SETTER_DEFINITION(DispatchResultCallback, mDispatchResultCallback)
};


template<typename ResultT, typename TargetParamsTupleT>
void DispatchManager<ResultT, TargetParamsTupleT>::handleEnqueuedDispatches()
{
    while (!mDispatches.empty())
    {
        const auto& tuple = mDispatches.front();
        dispatch(tuple);
        mDispatches.pop();
    }
}


template<typename ResultT, typename TargetParamsTupleT>
template<typename... TargetParamTs>
void DispatchManager<ResultT, TargetParamsTupleT>::performDispatch(TargetParamTs&&... args)
{
    dispatch(std::make_tuple<TargetParamTs...>(std::forward<TargetParamTs>(args)...));
}


template<typename ResultT, typename TargetParamsTupleT>
template<typename... TargetParamTs>
void DispatchManager<ResultT, TargetParamsTupleT>::enqueueDispatch(TargetParamTs&&... args)
{
    mDispatches.emplace(std::make_tuple(std::forward<TargetParamTs>(args)...));
}


template<typename ResultT, typename TargetParamsTupleT>
void DispatchManager<ResultT, TargetParamsTupleT>::dispatch(const TargetParamsTupleT& paramsTuple)
{
    for (const auto& callDispatchHandler : mTargetInvokers)
    {
        ResultT result;
        callDispatchHandler(&result, &paramsTuple);

        if (!mDispatchResultCallback(result))
        {
            break;
        }
    }
}


template<typename ResultT>
struct DispatchManager<ResultT, std::tuple<>> final : DispatchManagerBase
{
    using ResultCallback = std::function<bool(ResultT&)>;

    explicit DispatchManager(ResultCallback dispatchResultCallback = [](ResultT&) { return true; })
        : mDispatchResultCallback(std::move(dispatchResultCallback)) {}
    ~DispatchManager() override = default;

    void handleEnqueuedDispatches() override;

    void performDispatch();
    void enqueueDispatch();

private:
    bool mEnqueued = false;
    ResultCallback mDispatchResultCallback;

public:
    DEFAULT_SETTER_DEFINITION(DispatchResultCallback, mDispatchResultCallback)
};


template<typename ResultT>
void DispatchManager<ResultT, std::tuple<>>::handleEnqueuedDispatches()
{
    if (mEnqueued)
    {
        performDispatch();
        mEnqueued = false;
    }
}


template<typename ResultT>
void DispatchManager<ResultT, std::tuple<>>::performDispatch()
{
    for (const auto& callDispatchHandler : mTargetInvokers)
    {
        ResultT result;
        callDispatchHandler(&result, nullptr);

        if (!mDispatchResultCallback(result))
        {
            break;
        }
    }
}


template<typename ResultT>
void DispatchManager<ResultT, std::tuple<>>::enqueueDispatch()
{
    mEnqueued = true;
}


template<typename TargetParamsTupleT>
struct DispatchManager<void, TargetParamsTupleT> final : DispatchManagerBase
{
    void handleEnqueuedDispatches() override;

    template<typename... TargetParamTs>
    void performDispatch(TargetParamTs&&... args);

    template<typename... TargetParamTs>
    void enqueueDispatch(TargetParamTs&&... args);

private:
    void dispatch(const TargetParamsTupleT& paramsTuple);

    std::queue<TargetParamsTupleT> mDispatches;
};


template<typename TargetParamsTupleT>
void DispatchManager<void, TargetParamsTupleT>::handleEnqueuedDispatches()
{
    while (!mDispatches.empty())
    {
        const auto& tuple = mDispatches.front();
        dispatch(tuple);
        mDispatches.pop();
    }
}


template<typename TargetParamsTupleT>
template<typename... TargetParamTs>
void DispatchManager<void, TargetParamsTupleT>::performDispatch(TargetParamTs&&... args)
{
    dispatch(std::make_tuple<TargetParamTs...>(std::forward<TargetParamTs>(args)...));
}


template<typename TargetParamsTupleT>
template<typename... TargetParamTs>
void DispatchManager<void, TargetParamsTupleT>::enqueueDispatch(TargetParamTs&&... args)
{
    mDispatches.emplace(std::make_tuple(std::forward<TargetParamTs>(args)...));
}


template<typename TargetParamsTupleT>
void DispatchManager<void, TargetParamsTupleT>::dispatch(const TargetParamsTupleT& paramsTuple)
{
    for (const auto& callDispatchHandler : mTargetInvokers)
    {
        callDispatchHandler(nullptr, &paramsTuple);
    }
}


template<>
struct GRAPHEX_EXPORTABLE DispatchManager<void, std::tuple<>> final : DispatchManagerBase
{
    void handleEnqueuedDispatches() override;

    void performDispatch() const;
    void enqueueDispatch();

private:
    bool mEnqueued = false;
};

} // namespace GraphEx