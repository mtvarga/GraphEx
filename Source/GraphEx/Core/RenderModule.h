#pragma once

#include "../API/Module.h"
#include "CoreTypes.h"


namespace GraphEx::Core
{

struct GRAPHEX_EXPORTABLE RenderManager;


struct GRAPHEX_EXPORTABLE RenderModuleBase : Module, Requires<Container<RenderManager>>
{
    explicit RenderModuleBase(ModuleContainerBase* pContainer);

    virtual std::string getHumanReadableName() const = 0;
    virtual bool hasGlobalSettings() const;
    virtual void renderGlobalSettingsGui(Falcor::Gui::Widgets&);

    void update(Falcor::RenderContext*, const Falcor::ref<Falcor::Fbo>&) final;
};


template<typename ObjectT>
struct RenderModule : RenderModuleBase
{
    static_assert(std::is_base_of_v<SceneObject, ObjectT>, "RenderModule: ObjectT must be derived from SceneObject");

    using Object = ObjectT;

    explicit RenderModule(ModuleContainerBase* pContainer)
        : RenderModuleBase(pContainer) {}

    virtual void preRenderObject(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo, ObjectT& object) {}
    virtual void renderObject(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo, ObjectT& object) {}
    virtual void postRenderObject(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo, ObjectT& object) {}
};


template<>
struct RenderModule<SceneObject>;

} // namespace GraphEx::Core
