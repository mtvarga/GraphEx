#pragma once

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

#include <cereal/types/array.hpp>
#include <cereal/types/atomic.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/forward_list.hpp>
#include <cereal/types/functional.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/valarray.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>


#include "../../Utils/Standard.h"


// Place this in the translation unit where you register types
#define GRAPHEX_SERIALIZATION_TYPE_REGISTRY(unitName) \
    CEREAL_REGISTER_DYNAMIC_INIT(unitName)

// Place this in a header file that is definitely included before you first attempt to serialize anything
// The unitName provided as parameter should be the same name that you used in the GRAPHEX_SERIALIZATION_TYPE_REGISTRY macro
#define GRAPHEX_LOAD_SERIALIZATION_TYPE_REGISTRY(unitName) \
    CEREAL_FORCE_DYNAMIC_INIT(unitName)


// Care to place all calls of these in your program in a separate translation unit instead of header files
// The translation unit of your choice should include GraphEx.h or GraphEx/Serialization/Serialization.h and should start with
// the GRAPHEX_SERIALIZATION_TYPE_REGISTRY macro
#define GRAPHEX_REGISTER_SERIALIZABLE(...) \
    CEREAL_REGISTER_TYPE(__VA_ARGS__)

// Care to place all calls of these in your program in a separate translation unit instead of header files
// The translation unit of your choice should include GraphEx.h or GraphEx/Serialization/Serialization.h and should start with
// the GRAPHEX_SERIALIZATION_TYPE_REGISTRY macro
#define GRAPHEX_REGISTER_POLYMORPHIC_SERIALIZABLE(base, derived) \
    CEREAL_REGISTER_TYPE(derived) \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(base, derived)


// Make sure to place this in the global scope, outside any namespace!
#define GRAPHEX_SERIALIZATION_SPECIFY_BEGIN \
    namespace cereal {


// Make sure to place this in the global scope, after GRAPHEX_SERIALIZATION_SPECIFY_BEGIN
#define GRAPHEX_SERIALIZATION_SPECIFY_END \
    } // namespace cereal


// Make sure to place this in the global scope, between GRAPHEX_SERIALIZATION_SPECIFY_BEGIN and GRAPHEX_SERIALIZATION_SPECIFY_END
#define GRAPHEX_SERIALIZATION_SPECIFY_CONSTRUCT_BEGIN(...) \
    template<> struct LoadAndConstruct<__VA_ARGS__> {


#define GRAPHEX_SERIALIZATION_CONSTRUCT_FUNCTION_NAME \
    load_and_construct


// Make sure to place this in the global scope, between GRAPHEX_SERIALIZATION_CONSTRUCT_SPEC_BEGIN
// and GRAPHEX_SERIALIZATION_CONSTRUCT_SPEC_END
#define GRAPHEX_SERIALIZATION_CONSTRUCT_FUNCTION(archiveParamName, objectType, objectParamName) \
    template<typename Archive> \
    static void GRAPHEX_SERIALIZATION_CONSTRUCT_FUNCTION_NAME( \
        Archive& ar, \
        construct<objectType>& objectParamName \
    )


// Make sure to place this in the global scope, after GRAPHEX_SERIALIZATION_CONSTRUCT_FUNCTION, but before GRPAHEX_SERIALIZATION_SPECIFY_END
#define GRAPHEX_SERIALIZATION_SPECIFY_CONSTRUCT_END \
    };


// Make sure that the function defined with this is between GRAPHEX_SERIALIZATION_SPECIFY_BEGIN and GRAPHEX_SERIALIZATION_SPECIFY_END
// or inside a struct or a class that you intend to serialize
#define GRAPHEX_SERIALIZATION_FUNCTION_NAME \
    CEREAL_SERIALIZE_FUNCTION_NAME


// Make sure to place this in the global scope, between GRAPHEX_SERIALIZATION_MEMBER_FUNCTION and GRAPHEX_SERIALIZATION_SPECIFY_END
#define GRAPHEX_SERIALIZATION_FUNCTION(archiveParamName, objectType, objectParamName) \
    template<typename Archive> \
    void GRAPHEX_SERIALIZATION_FUNCTION_NAME( \
        Archive& archiveParamName, \
        objectType& objectParamName \
    )


// Make sure that the function defined with this is between GRAPHEX_SERIALIZATION_MEMBER_FUNCTION and GRAPHEX_SERIALIZATION_SPECIFY_END
// or inside a struct or a class that you intend to serialize
#define GRAPHEX_SERIALIZATION_SAVE_FUNCTION_NAME \
    CEREAL_SAVE_FUNCTION_NAME


// Make sure to place this in the global scope, between GRAPHEX_SERIALIZATION_SAVE_MEMBER_FUNCTION and GRAPHEX_SERIALIZATION_SPECIFY_END
#define GRAPHEX_SERIALIZATION_SAVE_FUNCTION(archiveParamName, objectType, objectParamName) \
    template<typename Archive> \
    void GRAPHEX_SERIALIZATION_SAVE_FUNCTION_NAME( \
        Archive& archiveParamName, \
        const objectType& objectParamName \
    )


// Make sure that the function defined with this is between GRAPHEX_SERIALIZATION_SAVE_MEMBER_FUNCTION and GRAPHEX_SERIALIZATION_SPECIFY_END
// or inside a struct or a class that you intend to serialize
#define GRAPHEX_SERIALIZATION_LOAD_FUNCTION_NAME \
    CEREAL_LOAD_FUNCTION_NAME


// Make sure to place this in the global scope, between GRAPHEX_SERIALIZATION_LOAD_MEMBER_FUNCTION and GRAPHEX_SERIALIZATION_SPECIFY_END
#define GRAPHEX_SERIALIZATION_LOAD_FUNCTION(archiveParamName, objectType, objectParamName) \
    template<typename Archive> \
    void GRAPHEX_SERIALIZATION_LOAD_FUNCTION_NAME( \
        Archive& archiveParamName, \
        objectType& objectParamName \
    )


#define GRAPHEX_SERIALIZE_WITH_NAME(...) \
    CEREAL_NVP(__VA_ARGS__)
