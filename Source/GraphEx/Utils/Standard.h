#pragma once

#include <list>
#include <forward_list>
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <stdexcept>
#include <algorithm>
#include <queue>
#include <functional>
#include <filesystem>
#include <charconv>
#include <fstream>
#include <iostream>
#include <array>
#include <sstream>

#include <Falcor.h>


#define DEFAULT_CONST_GETREF_DEFINITION(name, member, ...) \
    __VA_ARGS__ auto get##name() const -> std::add_lvalue_reference_t<std::add_const_t<decltype(member)>>\
    {\
        return member;\
    }

#define DEFAULT_GETREF_DEFINITION(name, member, ...) \
    __VA_ARGS__ auto get##name() -> std::add_lvalue_reference_t<decltype(member)>\
    {\
        return member;\
    }

#define DEFAULT_CONST_GETTER_DEFINITION(name, member, ...) \
    __VA_ARGS__ auto get##name() const -> std::add_const_t<decltype(member)>\
    {\
        return member;\
    }

#define DEFAULT_GETTER_DEFINITION(name, member, ...) \
    __VA_ARGS__ auto get##name() -> decltype(member)\
    {\
        return member;\
    }

#define DEFAULT_CONST_NONCONST_GETREF_DEFINITIONS(name, member, ...) \
    DEFAULT_CONST_GETREF_DEFINITION(name, member, __VA_ARGS__)\
    DEFAULT_GETREF_DEFINITION(name, member, __VA_ARGS__)

#define DEFAULT_CONST_NONCONST_GETTER_DEFINITIONS(name, member, ...) \
    DEFAULT_CONST_GETTER_DEFINITION(name, member, __VA_ARGS__)\
    DEFAULT_GETTER_DEFINITION(name, member, __VA_ARGS__)

#define DEFAULT_SETTER_DEFINITION(name, member, ...) \
    __VA_ARGS__ void set##name(decltype(member) value)\
    {\
        member = std::move(value);\
    }

#define DEFAULT_CONST_NONCONST_GETTER_SETTER_DEFINITIONS(name, member, ...) \
    DEFAULT_CONST_NONCONST_GETTER_DEFINITIONS(name, member, __VA_ARGS__)\
    DEFAULT_SETTER_DEFINITION(name, member, __VA_ARGS__)

#define DEFAULT_CONST_NONCONST_GETREF_SETTER_DEFINITIONS(name, member, ...) \
    DEFAULT_CONST_NONCONST_GETREF_DEFINITIONS(name, member, __VA_ARGS__)\
    DEFAULT_SETTER_DEFINITION(name, member, __VA_ARGS__)

#define DEFAULT_CONST_GETTER_SETTER_DEFINITION(name, member, ...) \
    DEFAULT_CONST_GETTER_DEFINITION(name, member, __VA_ARGS__)    \
    DEFAULT_SETTER_DEFINITION(name, member, __VA_ARGS__)

#define DEFAULT_CONST_GETREF_SETTER_DEFINITION(name, member, ...) \
    DEFAULT_CONST_GETREF_DEFINITION(name, member, __VA_ARGS__)    \
    DEFAULT_SETTER_DEFINITION(name, member, __VA_ARGS__)


#define MAKE_MOVE_ONLY(class) \
    class(const class&) = delete;\
    class& operator=(const class&) = delete;

#define DEFAULT_MOVE_SEMANTICS(class) \
    class(class&&) noexcept = default;\
    class& operator=(class&&) noexcept = default;


#if FALCOR_WINDOWS
#define GRAPHEX_EXPORT
#define GRAPHEX_IMPORT
#elif FALCOR_LINUX
#define GRAPHEX_EXPORT __attribute__((visibility("default")))
#define GRAPHEX_IMPORT
#endif


#ifdef GRAPHEX_EXPORT_EXPORTABLES
#define GRAPHEX_EXPORTABLE GRAPHEX_EXPORT
#else // GRAPHEX_EXPORT_EXPORTABLES
#define GRAPHEX_EXPORTABLE GRAPHEX_IMPORT
#endif // GRAPHEX_EXPORT_EXPORTABLES


namespace GraphEx
{

using TypeId = std::type_index;

template<typename T>
static TypeId GetTypeId()
{
    return { typeid(T) };
}

using ModuleId = std::string;
using ModuleContainerId = std::string;

}
