#pragma once

#include "SerializationMacros.h"


GRAPHEX_SERIALIZATION_SPECIFY_BEGIN

template<typename Archive, typename T, int N>
void GRAPHEX_SERIALIZATION_FUNCTION_NAME(Archive& ar, Falcor::math::vector<T, N>& vector)
{
    for (int i = 0; i < N; ++i)
    {
        ar(vector[i]);
    }
}

GRAPHEX_SERIALIZATION_SPECIFY_END
