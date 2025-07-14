#include "SerializationManager.h"


using namespace GraphEx::Internal;


std::optional<GraphEx::InputArchive> SerializationManager::beginLoad(std::istream& is)
{
    try
    {
        return { is };
    }
    catch (...)
    {
        // Most possibly encountered JSON syntax error
        return std::nullopt;
    }
}


std::optional<GraphEx::OutputArchive> SerializationManager::beginSave(std::ostream& os)
{
    try
    {
        return { os };
    }
    catch (...)
    {
        // This should not really happen (perhaps could not open stream?)
        return std::nullopt;
    }
}


void SerializationManager::finish()
{
    while (!mInvalidityListStack.empty())
    {
        mInvalidityListStack.pop();
    }

    releaseTrackedRefs();
}


void SerializationManager::addTrackedFalcorRef(const uintptr_t address, Falcor::ref<Falcor::Object> ref)
{
    mTrackedRefs.emplace(address, std::move(ref));
}


bool SerializationManager::isFalcorRefTracked(uintptr_t address) const
{
    return mTrackedRefs.find(address) != mTrackedRefs.end();
}


Falcor::ref<Falcor::Object> SerializationManager::getTrackedFalcorRef(uintptr_t address) const
{
    if (const auto it = mTrackedRefs.find(address); it != mTrackedRefs.end())
    {
        return it->second;
    }

    FALCOR_THROW("SerializationHelper: Attempted to retrieve an untracked object.");
}


void SerializationManager::releaseTrackedRefs()
{
    mTrackedRefs.clear();
}


void SerializationManager::pushInvalidityList()
{
    mInvalidityListStack.emplace(std::make_shared<InvalidityList>());
}


std::shared_ptr<SerializationManager::InvalidityList> SerializationManager::popInvalidityList()
{
    const auto result = mInvalidityListStack.top();
    mInvalidityListStack.pop();
    return result;
}


void SerializationManager::logInvalidity(const InvalidityMessage& message)
{
    if (mInvalidityListStack.empty())
    {
        Falcor::logWarning("Attempted to log an invalidity message, but no invalidity list was pushed. This is likely a bug.");
        return;
    }

    mInvalidityListStack.top()->push_back(message);
}


SerializationManager& SerializationManager::get()
{
    static SerializationManager instance;
    return instance;
}
