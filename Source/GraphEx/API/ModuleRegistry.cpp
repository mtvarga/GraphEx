#include "ModuleRegistry.h"

#include "Module.h"


using namespace GraphEx;


bool ModuleRegistry::hasContainer(const ModuleContainerId& containerId) const
{
    return mModuleIdsForContainer.find(containerId) != mModuleIdsForContainer.end();
}


auto ModuleRegistry::getModuleIdsForContainer(const ModuleContainerId& containerId) const -> std::optional<std::reference_wrapper<const ModuleIds>>
{
    return hasContainer(containerId) ? std::optional{ std::cref(mModuleIdsForContainer.at(containerId)) } : std::nullopt;
}


auto ModuleRegistry::getModuleIdsForContainerMutable(const ModuleContainerId& containerId) -> ModuleIds&
{
    return mModuleIdsForContainer[containerId];
}


bool ModuleRegistry::hasModule(const ModuleId& moduleId) const
{
    return mModules.find(moduleId) != mModules.end();
}


auto ModuleRegistry::getModule(const ModuleId& moduleId) const -> ModulePtr<Module>
{
    return hasModule(moduleId) ? mModules.at(moduleId) : nullptr;
}


bool ModuleRegistry::containerHasModule(const ModuleContainerId& containerId, const ModuleId& moduleId) const
{
    if (!hasModule(moduleId))
    {
        return false;
    }

    const auto& moduleIds = getModuleIdsForContainer(containerId);
    return moduleIds && moduleIds->get().find(moduleId) != moduleIds->get().end();
}

auto ModuleRegistry::getModuleForContainer(const ModuleContainerId& containerId, const ModuleId& moduleId) const -> ModulePtr<Module>
{
    return containerHasModule(containerId, moduleId) ? getModule(moduleId) : nullptr;
}


std::optional<ModuleId> ModuleRegistry::getModuleIdForTypeId(const TypeId& typeId) const
{
    return mModuleIdForTypeId.find(typeId) != mModuleIdForTypeId.end() ? std::optional{mModuleIdForTypeId.at(typeId)} : std::nullopt;
}


void ModuleRegistry::cleanup()
{
    mModules.clear();
    mModuleIdsForContainer.clear();
    mModuleIdForTypeId.clear();
    mModuleStateSerializers.clear();
}


ModuleRegistry& ModuleRegistry::get()
{
    static std::unique_ptr<ModuleRegistry> pInstance(new ModuleRegistry);
    return *pInstance;
}
