#pragma once

#include "../Serialization/Serialization.h"
#include "../Utils/Standard.h"


namespace GraphEx
{

// Forward declare Module as Module.h includes this header file
struct GRAPHEX_EXPORTABLE Module;


class GRAPHEX_EXPORTABLE ModuleRegistry final
{
    template<typename ModuleT>
    using ModulePtr = std::shared_ptr<ModuleT>;
    using ModuleIds = std::unordered_set<ModuleId>;
    using ModuleDictionary = std::unordered_map<ModuleId, ModulePtr<Module>>;
    using ModuleTypeDictionary = std::unordered_map<TypeId, ModuleId>;
    using ModuleContainerDictionary = std::unordered_map<ModuleContainerId, ModuleIds>;

    using SerializedModuleState = TaggedPolymorphicSafeAnchor<ModuleId, std::shared_ptr<ModuleState>>;
    using ModuleStateStore = std::vector<SerializedModuleState>;
    using ModuleStateSerializerDictionary = std::unordered_map<ModuleId, std::unique_ptr<Internal::ModuleStateSerializerBase>>;

public:
    MAKE_MOVE_ONLY(ModuleRegistry)
    DEFAULT_MOVE_SEMANTICS(ModuleRegistry)

    template<typename ModuleT, typename... Args, typename = std::enable_if_t<std::is_base_of_v<Module, ModuleT>>>
    ModulePtr<ModuleT> registerModuleForContainer(const ModuleContainerId& containerId, Args&&... args);
    ModulePtr<Module> getModuleForContainer(const ModuleContainerId& containerId, const ModuleId& moduleId) const;
    bool containerHasModule(const ModuleContainerId& containerId, const ModuleId& moduleId) const;
    std::optional<std::reference_wrapper<const ModuleIds>> getModuleIdsForContainer(const ModuleContainerId& containerId) const;
    std::optional<ModuleId> getModuleIdForTypeId(const TypeId& typeId) const;

    template<typename Archive>
    void saveModuleStates(Archive& ar) const;

    template<typename Archive>
    void loadModuleStates(Archive& ar) const;

    void cleanup();

    static ModuleRegistry& get();

private:
    ModuleRegistry() = default;

    bool hasContainer(const ModuleContainerId& containerId) const;

    template<typename ModuleT, typename... Args>
    std::tuple<ModuleId, ModulePtr<ModuleT>> registerModule(std::true_type, Args&&... args);

    template<typename ModuleT, typename... Args>
    std::tuple<ModuleId, ModulePtr<ModuleT>> registerModule(std::false_type, Args&&... args);

    bool hasModule(const ModuleId& moduleId) const;
    ModulePtr<Module> getModule(const ModuleId& moduleId) const;

    ModuleIds& getModuleIdsForContainerMutable(const ModuleContainerId& containerId);

    ModuleDictionary mModules;
    ModuleContainerDictionary mModuleIdsForContainer;
    ModuleTypeDictionary mModuleIdForTypeId;

    ModuleStateSerializerDictionary mModuleStateSerializers;
};


template<typename ModuleT, typename... Args, typename>
auto ModuleRegistry::registerModuleForContainer(const ModuleContainerId& containerId, Args&&... args) -> ModulePtr<ModuleT>
{
    const auto [moduleId, pModule] = registerModule<ModuleT>(Internal::IsModuleSerializable<ModuleT>, std::forward<Args>(args)...);
    getModuleIdsForContainerMutable(containerId).insert(moduleId);
    return pModule;
}


template<typename Archive>
void ModuleRegistry::saveModuleStates(Archive& ar) const
{
    ModuleStateStore moduleStateStore;

    for (const auto& [ moduleId, serializer ] : mModuleStateSerializers)
    {
        moduleStateStore.emplace_back(moduleId, serializer->getStateBase());
    }

    try
    {
        ar(SaveNamed("moduleStates", moduleStateStore));
    }
    catch (...)
    {
        Falcor::logError("Failed to save module states to project file. Check surrounding logs.");
        throw;
    }

    for (const auto& serializedState : moduleStateStore)
    {
        const auto& [ moduleId, pStateBase ] = serializedState.get();

        if (!serializedState.success())
        {
            Falcor::logError("Could not safely serialize module state for module '{}'. This should not happen. "
                             "If you declared a module with a serializable state using SerializableModuleState, make sure to register "
                             "the state by using the GRAPHEX_REGISTER_MODULE_STATE macro!", moduleId);
        }
    }
}


template<typename Archive>
void ModuleRegistry::loadModuleStates(Archive& ar) const
{
    auto hadIssues = false;

    ModuleStateStore moduleStateStore;

    try
    {
        ar(LoadNamed("moduleStates", moduleStateStore));
    }
    catch (...)
    {
        Falcor::logError("Failed to load module states from project file. Check surrounding logs.");
        throw;
    }

    for (const auto& serializedState : moduleStateStore)
    {
        const auto& [ moduleId, pStateBase ] = serializedState.get();

        if (!serializedState.success())
        {
            hadIssues = true;
            Falcor::logError("Could not load module state for module '{}'. The state for this module was unknown for the serialization "
                             "framework or the written state was invalid. Semi-compatible project file? Forgot to register module state?",
                             moduleId);
            continue;
        }

        if (!hasModule(moduleId))
        {
            hadIssues = true;
            Falcor::logError("Could not load state for module with ID '{}'. This module is not currently registered.",
                             moduleId);
            continue;
        }

        if (mModuleStateSerializers.find(moduleId) == mModuleStateSerializers.end())
        {
            hadIssues = true;
            Falcor::logError("Could not find serializer for module '{}'. This should not happen. The state will not be loaded for this "
                               "module!", moduleId);
        }

        const auto& serializer = mModuleStateSerializers.at(moduleId);
        serializer->setStateBase(pStateBase);
    }

    if (hadIssues)
    {
        Falcor::logWarning("Some module states were not loaded properly. This may be because a project file was loaded that is not directly "
                           "compatible with the module set of the current running version of the program, or because you forgot to register "
                           "some module states with the GRAPHEX_REGISTER_MODULE_STATE macro, or maybe because you forgot to declare that "
                           "the module has a serializable state by deriving from SerializableModuleState.");
    }
}


template<typename ModuleT, typename... Args>
auto ModuleRegistry::registerModule(std::false_type, Args&&... args) -> std::tuple<ModuleId, ModulePtr<ModuleT>>
{
    static_assert(std::is_base_of_v<Module, ModuleT>, "ModuleRegistry: ModuleT type must be derived from Module.");
    static_assert(!std::is_abstract_v<ModuleT>, "ModuleRegistry: ModuleT type must be abstract.");
    static_assert(std::is_constructible_v<ModuleT, Args...>,
                  "ModuleRegistry: there is no valid constructor to call for ModuleT with given arguments.");

    const auto pModule = std::make_shared<ModuleT>(std::forward<Args>(args)...);
    const auto moduleId = std::static_pointer_cast<Module>(pModule)->getModuleId();  // If subclasses declare getModuleId() as private,
    const TypeId moduleTypeId = typeid(ModuleT);                                     // it would be inaccessible here, so we cast to base

    if (hasModule(moduleId))
    {
        FALCOR_THROW(
            "Attempted register a Module with an ID that is already associated with another module: '{}' "
            "Module IDs must be unqiue. This is illegal",
            moduleId
        );
    }

    mModules.emplace(moduleId, pModule);
    mModuleIdForTypeId.emplace(moduleTypeId, moduleId);

    return std::make_tuple(moduleId, pModule);
}


template<typename ModuleT, typename... Args>
auto ModuleRegistry::registerModule(std::true_type, Args&&... args) -> std::tuple<ModuleId, ModulePtr<ModuleT>>
{
    const auto result = registerModule<ModuleT>(std::false_type{ }, std::forward<Args>(args)...);
    const auto& [ moduleId, pModule ] = result;

    // Additionally, register serialization mechanism
    mModuleStateSerializers.emplace(
        moduleId,
        std::make_unique<Internal::ModuleStateSerializer<ModuleT, typename ModuleT::StateType>>(pModule)
    );

    return result;
}

} // namespace GraphEx
