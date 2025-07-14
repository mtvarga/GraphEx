#pragma once

#include "SerializationManager.h"


GRAPHEX_SERIALIZATION_SPECIFY_BEGIN

template <typename Archive, typename T, typename = std::enable_if_t<std::is_copy_constructible_v<T>>>
void GRAPHEX_SERIALIZATION_SAVE_FUNCTION_NAME(Archive& ar, const Falcor::ref<T>& falcorRef)
{
    const auto ref = std::shared_ptr<T>(falcorRef.get(), [](const auto&) { });  // shared_ptr will not delete object
    ar(GRAPHEX_SERIALIZE_WITH_NAME(ref));
}


template <typename Archive, typename T, typename = std::enable_if_t<std::is_copy_constructible_v<T>>>
void GRAPHEX_SERIALIZATION_LOAD_FUNCTION_NAME(Archive& ar, Falcor::ref<T>& falcorRef)
{
    std::shared_ptr<T> ref;
    ar(GRAPHEX_SERIALIZE_WITH_NAME(ref));

    auto&      serializationManager = GraphEx::Internal::SerializationManager::get();
    const auto address = reinterpret_cast<uintptr_t>(ref.get());

    if (serializationManager.isFalcorRefTracked(address))
    {
        falcorRef = Falcor::static_ref_cast<T>(serializationManager.getTrackedFalcorRef(address));
        return;
    }

    serializationManager.addTrackedFalcorRef(address, falcorRef = Falcor::make_ref<T>(*ref));
}

GRAPHEX_SERIALIZATION_SPECIFY_END
