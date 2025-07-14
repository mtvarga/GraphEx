#pragma once

#include "SerializationMacros.h"


namespace GraphEx
{

struct GRAPHEX_EXPORTABLE ModuleState;


template<typename ModuleStateT>
struct HasSerializableState;


namespace Internal
{

// Do not inherit directly from this struct!
// If you want your module to be serializable, inherit from the templated SerializableModuleState instead, and provide it your state type
struct GRAPHEX_EXPORTABLE HasSerializableStateTag {};


template<typename ModuleT>
constexpr auto IsModuleSerializable = std::is_base_of<HasSerializableStateTag, ModuleT>{ };


struct GRAPHEX_EXPORTABLE ModuleStateSerializerBase
{
    virtual ~ModuleStateSerializerBase() = default;

    virtual void setStateBase(const std::shared_ptr<ModuleState>& pState) const = 0;
    virtual std::shared_ptr<ModuleState> getStateBase() const = 0;
};


template<typename ModuleT, typename StateT>
struct ModuleStateSerializer final : ModuleStateSerializerBase
{
    using Module = ModuleT;
    using State = StateT;

    static_assert(
        std::is_base_of_v<HasSerializableState<State>, Module>,
        "ModuleStateSerializer: Module must define respective state, same as ModuleStateT"
    );

    static_assert(std::is_base_of_v<ModuleState, State>, "ModuleStateSerializer: ModuleStateT must be derived from ModuleState");

    explicit ModuleStateSerializer(std::shared_ptr<Module> pModule);

private:
    void setStateBase(const std::shared_ptr<ModuleState>& pState) const override;
    std::shared_ptr<ModuleState> getStateBase() const override;

    std::shared_ptr<Module> mpModule;
};


template<typename ModuleT, typename StateT>
ModuleStateSerializer<ModuleT, StateT>::ModuleStateSerializer(std::shared_ptr<Module> pModule)
    : mpModule(std::move(pModule)) {}


template<typename ModuleT, typename StateT>
void ModuleStateSerializer<ModuleT, StateT>::setStateBase(const std::shared_ptr<ModuleState>& pState) const
{
    if (const auto pStateDowncast = std::dynamic_pointer_cast<State>(pState))
    {
        mpModule->setState(pStateDowncast);
        return;
    }

    FALCOR_THROW("State type mismatch when loading state.");
}


template<typename ModuleT, typename StateT>
std::shared_ptr<ModuleState> ModuleStateSerializer<ModuleT, StateT>::getStateBase() const
{
    return mpModule->getState();
}

} // namespace GraphEx::Internal
} // namespace GraphEx


// Make sure to place this in the global scope, outside any namespace!
#define GRAPHEX_REGISTER_MODULE_STATE(...) \
    GRAPHEX_REGISTER_SERIALIZABLE(__VA_ARGS__) \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(GraphEx::ModuleState, __VA_ARGS__)
