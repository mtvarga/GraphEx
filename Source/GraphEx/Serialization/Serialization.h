#pragma once


// This header file ensures that resources for serialization are all included and in the proper order!
// Do not care to include any of the internal files in your own code as it may break the underlying serialization framework,
//    only include this header file!

#include "../Utils/Standard.h"

#include "Internal/SerializationMacros.h"
#include "Internal/SerializationManager.h"
#include "Internal/SerializationTemplates.h"

#include "Internal/ModuleSerialization.h"
#include "Internal/ReferenceSerialization.h"
#include "Internal/VectorSerialization.h"
#include "Internal/CameraSerialization.h"
