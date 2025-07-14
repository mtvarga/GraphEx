#pragma once

#include "ModuleRegistry.h"


namespace GraphEx
{

struct GRAPHEX_EXPORTABLE ModuleContainerBase
{
    virtual ~ModuleContainerBase() = default;

    template<typename ModuleT>
    bool contains(ModuleId moduleId = "") const;

    template<typename... ModuleT>
    bool containsAny() const;

    template<typename... ModuleT>
    bool containsAll() const;

    template<typename ModuleT>
    std::shared_ptr<ModuleT> getMaybeContained(ModuleId moduleId = "") const;

    template<typename... ModuleTs>
    std::tuple<std::shared_ptr<ModuleTs>...> getsMaybeContained() const;

    template<typename ModuleT>
    ModuleT& getContained(const ModuleId& moduleId = "") const;

    template<typename... ModuleTs>
    std::tuple<ModuleTs&...> getsContained() const;

    virtual ModuleContainerId getModuleContainerId() const = 0;
};


template<typename ModuleT>
bool ModuleContainerBase::contains(ModuleId moduleId) const
{
    if (const auto maybeId = ModuleRegistry::get().getModuleIdForTypeId(typeid(ModuleT));
        moduleId.empty() && maybeId)
    {
        moduleId = *maybeId;
    }

    return ModuleRegistry::get().containerHasModule(getModuleContainerId(), moduleId);
}


template<typename... ModuleT>
bool ModuleContainerBase::containsAny() const
{
    const auto results = {contains<ModuleT>()...};
    return std::any_of(results.begin(), results.end(), [](const auto i) { return i; });
}


template<typename... ModuleT>
bool ModuleContainerBase::containsAll() const
{
    const auto results = {contains<ModuleT>()...};
    return std::all_of(results.begin(), results.end(), [](const auto i) { return i; });
}


template<typename ModuleT>
std::shared_ptr<ModuleT> ModuleContainerBase::getMaybeContained(ModuleId moduleId) const
{
    if (const auto maybeId = ModuleRegistry::get().getModuleIdForTypeId(typeid(ModuleT));
        moduleId.empty() && maybeId)
    {
        moduleId = *maybeId;
    }

    return std::static_pointer_cast<ModuleT>(ModuleRegistry::get().getModuleForContainer(getModuleContainerId(), moduleId));
}


template<typename... ModuleTs>
std::tuple<std::shared_ptr<ModuleTs>...> ModuleContainerBase::getsMaybeContained() const
{
    return std::make_tuple(getMaybeContained<ModuleTs>()...);
}


template<typename ModuleT>
ModuleT& ModuleContainerBase::getContained(const ModuleId& moduleId) const
{
    if (const auto module = getMaybeContained<ModuleT>(moduleId))
    {
        return *module;
    }

    FALCOR_THROW("Attempted to retrieve an unregistered module '{}' in container '{}'", moduleId, getModuleContainerId());
}


template<typename... ModuleTs>
std::tuple<ModuleTs&...> ModuleContainerBase::getsContained() const
{
    return std::tie(getContained<ModuleTs>()...);
}


struct GRAPHEX_EXPORTABLE Module
{
    explicit Module(ModuleContainerBase*) {}
    virtual ~Module() = default;

    virtual void init(Falcor::RenderContext* pRenderContext) = 0;
    virtual void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) = 0;
    virtual void cleanup() = 0;

    virtual ModuleId getModuleId() const = 0;
};


template<typename ModuleBaseT>
struct ModuleContainer : ModuleContainerBase
{
    static_assert(std::is_base_of_v<Module, ModuleBaseT>, "BaseModuleT for ModuleContainer must inherit from Module");

    template<typename ModuleT, typename... Args>
    void registerModule(Args&&... args);

protected:
    virtual void onModuleRegistered(const std::shared_ptr<ModuleBaseT>& pModule);
    virtual std::vector<std::shared_ptr<ModuleBaseT>> getAllModules() const;
};


template<typename ModuleBaseT>
template<typename ModuleT, typename... Args>
void ModuleContainer<ModuleBaseT>::registerModule(Args&&... args)
{
    static_assert(std::is_base_of_v<ModuleBaseT, ModuleT>, "Module type must be derived from the base module type of the container.");

    onModuleRegistered(
        ModuleRegistry::get().registerModuleForContainer<ModuleT>(
            getModuleContainerId(), static_cast<ModuleContainerBase*>(this), std::forward<Args>(args)...
        )
    );
}


template<typename ModuleBaseT>
void ModuleContainer<ModuleBaseT>::onModuleRegistered(const std::shared_ptr<ModuleBaseT>& pModule)
{
    Falcor::logInfo("Successfully registered module '{}' in container '{}'", pModule->getModuleId(), getModuleContainerId());
}


template<typename ModuleBaseT>
std::vector<std::shared_ptr<ModuleBaseT>> ModuleContainer<ModuleBaseT>::getAllModules() const
{
    if (const auto myModulesWrapped = ModuleRegistry::get().getModuleIdsForContainer(getModuleContainerId()))
    {
        const auto& myModules = myModulesWrapped->get();

        std::vector<std::shared_ptr<ModuleBaseT>> result;
        result.reserve(myModules.size());

        for (const auto& moduleId : myModules)
        {
            result.emplace_back(
                std::static_pointer_cast<ModuleBaseT>(ModuleRegistry::get().getModuleForContainer(getModuleContainerId(), moduleId))
            );
        }

        return result;
    }

    return {};
}


template<typename ModuleBaseT>
struct ContainerModule : Module, ModuleContainer<ModuleBaseT>
{
    explicit ContainerModule(ModuleContainerBase* pContainer);
    ModuleContainerId getModuleContainerId() const override;
};


template<typename ModuleBaseT>
ContainerModule<ModuleBaseT>::ContainerModule(ModuleContainerBase* pContainer)
    : Module(pContainer) {}


template<typename ModuleBaseT>
ModuleContainerId ContainerModule<ModuleBaseT>::getModuleContainerId() const
{
    return getModuleId();
}


template<typename... ModuleTs>
struct Associated {};


template<typename ContainerT>
struct Container {};


template<typename... ModuleTs>
struct Siblings {};


template<typename... RequirementTs>
struct Requires;


template<typename ContainerT, typename... RequirementTs>
struct Requires<Container<ContainerT>, RequirementTs...> : Requires<RequirementTs...>
{
    explicit Requires(ModuleContainerBase* pContainerBase);

protected:
    auto getsRequired() const -> decltype(auto);

    ContainerT* mpContainer;
};


template<typename ContainerT, typename... RequirementTs>
Requires<Container<ContainerT>, RequirementTs...>::Requires(ModuleContainerBase* pContainerBase)
    : Requires<RequirementTs...>(pContainerBase)
{
    mpContainer = dynamic_cast<ContainerT*>(pContainerBase);  // We use dynamic_cast here because container might not be correct

    if (!mpContainer)
    {
        FALCOR_THROW("Module container requirements were not met. Got module container with ID '{}', "
                     "which is not the required module container", pContainerBase->getModuleContainerId());
    }
}


template<typename ContainerT, typename... RequirementTs>
auto Requires<Container<ContainerT>, RequirementTs...>::getsRequired() const -> decltype(auto)
{
    return Requires<RequirementTs...>::getsRequired();
}


template<typename... ModuleTs, typename... RequirementTs>
struct Requires<Associated<ModuleTs...>, RequirementTs...> : Requires<RequirementTs...>
{
    explicit Requires(ModuleContainerBase* pContainer, ModuleTs&... associatedModules);

private:
    std::tuple<ModuleTs&...> mRequiredModules;

protected:
    template<typename ModuleT>
    ModuleT& getRequired() const;
    auto getsRequired() const -> decltype(auto);
};


template<typename... ModuleTs, typename... RequirementTs>
Requires<Associated<ModuleTs...>, RequirementTs...>::Requires(ModuleContainerBase* pContainer, ModuleTs&... associatedModules)
    : Requires<RequirementTs...>(pContainer), mRequiredModules(std::tie(associatedModules...)) {}


template<typename... ModuleTs, typename... RequirementTs>
template<typename ModuleT>
ModuleT& Requires<Associated<ModuleTs...>, RequirementTs...>::getRequired() const
{
    auto superRequirements = Requires<RequirementTs...>::getsRequired();
    // If the compiler failed here, it means you defined multiple types of dependencies to the same module.
    // This is illegal in terms of the module tree -- consider simplifying your dependencies
    return std::get<ModuleT&>(std::tuple_cat(superRequirements, mRequiredModules));
}


template<typename... ModuleTs, typename... RequirementTs>
auto Requires<Associated<ModuleTs...>, RequirementTs...>::getsRequired() const -> decltype(auto)
{
    auto superRequirements = Requires<RequirementTs...>::getsRequired();
    return std::tuple_cat(superRequirements, mRequiredModules);
}


template<typename... ModuleTs, typename... RequirementTs>
struct Requires<Siblings<ModuleTs...>, RequirementTs...> : Requires<RequirementTs...>
{
    explicit Requires(ModuleContainerBase* pContainer);

protected:
    template<typename ModuleT>
    ModuleT& getRequired() const;
    auto getsRequired() const -> decltype(auto);
};


template<typename... ModuleTs, typename... RequirementTs>
Requires<Siblings<ModuleTs...>, RequirementTs...>::Requires(ModuleContainerBase* pContainer)
    : Requires<RequirementTs...>(pContainer)
{
    if (!pContainer->containsAll<ModuleTs...>())
    {
        FALCOR_THROW("Not all required modules are present in the container for a module.");
    }
}


template<typename... ModuleTs, typename... RequirementTs>
template<typename ModuleT>
ModuleT& Requires<Siblings<ModuleTs...>, RequirementTs...>::getRequired() const
{
    auto superRequirements = Requires<RequirementTs...>::getsRequired();
    auto myRequirements = this->mpContainer->template getsContained<ModuleTs...>();
    // If the compiler failed here, it means you defined multiple types of dependencies to the same module.
    // This is illegal in terms of the module tree -- consider simplifying your dependencies
    // Another reason why this could fail is if you try to get
    return std::get<ModuleT&>(std::tuple_cat(superRequirements, myRequirements));
}


template<typename... ModuleTs, typename... RequirementTs>
auto Requires<Siblings<ModuleTs...>, RequirementTs...>::getsRequired() const -> decltype(auto)
{
    auto superRequirements = Requires<RequirementTs...>::getsRequired();
    auto myRequirements = this->mpContainer->template getsContained<ModuleTs...>();
    return std::tuple_cat(superRequirements, myRequirements);
}


template<>
struct GRAPHEX_EXPORTABLE Requires<>
{
    explicit Requires(ModuleContainerBase*);

protected:
    ModuleContainerBase* mpContainer;

    auto getsRequired() const -> decltype(auto);
};


inline Requires<>::Requires(ModuleContainerBase* pContainer)
    : mpContainer(pContainer) {}


inline auto Requires<>::getsRequired() const -> decltype(auto)
{
    return std::tuple{};
}


struct GRAPHEX_EXPORTABLE ModuleState
{
    virtual ~ModuleState() = default;

    template<typename Archive>
    void serialize(Archive&) { }
};


template<typename ModuleStateT>
struct HasSerializableState : Internal::HasSerializableStateTag
{
    static_assert(std::is_base_of_v<ModuleState, ModuleStateT>, "HasSerializableState: ModuleStateT must be derived from ModuleState");
    static_assert(std::is_default_constructible_v<ModuleStateT>, "HasSerializableState: ModuleStateT must be default constructible");

    HasSerializableState();
    virtual ~HasSerializableState() = default;

    virtual void setState(std::shared_ptr<ModuleStateT> pState);
    virtual const std::shared_ptr<ModuleStateT>& getState() const;

protected:
    std::shared_ptr<ModuleStateT> mpState;

private:
    // For ModuleRegistry, to access ModuleStateT
    using StateType = ModuleStateT;

    friend class ModuleRegistry;
};


template<typename ModuleStateT>
HasSerializableState<ModuleStateT>::HasSerializableState()
    : mpState(std::make_shared<ModuleStateT>()) {}


template<typename ModuleStateT>
void HasSerializableState<ModuleStateT>::setState(std::shared_ptr<ModuleStateT> pState)
{
    mpState = std::move(pState);
}


template<typename ModuleStateT>
const std::shared_ptr<ModuleStateT>& HasSerializableState<ModuleStateT>::getState() const
{
    return mpState;
}

} // namespace GraphEx



