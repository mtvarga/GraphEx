#pragma once

#include "../Application.h"
#include "../Utils/ProgramWrapper.h"

#include "CameraManager.h"
#include "SceneManager.h"
#include "RenderModule.h"


namespace GraphEx::Core
{

struct GRAPHEX_EXPORTABLE RenderManagerState final : ModuleState
{
    bool vSync = false;
    Falcor::float4 backgroundColor{0.15f, 0.15f, 0.15f, 1.0f};
    std::vector<SerializedSceneObjectState> orderedObjectStates;

    uint32_t selectedRendererIndex = 0;

    template<typename Archive>
    void serialize(Archive& ar);
};


template<typename Archive>
void RenderManagerState::serialize(Archive& ar)
{
    ar(GRAPHEX_SERIALIZE_WITH_NAME(vSync));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(backgroundColor));
    ar(SerializeNamed<Archive>("selectedRenderer", selectedRendererIndex));
    ar(SerializeNamed<Archive>("objects", orderedObjectStates));
}


struct GRAPHEX_EXPORTABLE RenderManager final : ContainerModule<RenderModuleBase>,
                                         Requires<Container<Application>, Siblings<SceneManager, CameraManager>>,
                                         HasSerializableState<RenderManagerState>
{
    explicit RenderManager(ModuleContainerBase* pContainer);

    void init(Falcor::RenderContext* pRenderContext) override;
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override;
    void cleanup() override;

    ModuleId getModuleId() const override;

    void setState(std::shared_ptr<RenderManagerState> pState) override;
    const std::shared_ptr<RenderManagerState>& getState() const override;

    void onSceneObjectAdded(const std::shared_ptr<SceneObject>& pSceneObject);
    void onSceneObjectRemoved(const std::shared_ptr<SceneObject>& pSceneObject);

    Falcor::ref<const Falcor::Camera> getActiveCamera() const;

    void setBuiltinRenderVars(const Falcor::ShaderVar& rootVar, const SceneObject* pSceneObject = nullptr) const;

    template<typename RendererT, typename RenderableT>
    void preRenderObject(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo, RenderableT& renderable) const;

    template<typename RendererT, typename RenderableT>
    void renderObject(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo, RenderableT& renderable) const;

    template<typename RendererT, typename RenderableT>
    void postRenderObject(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo, RenderableT& renderable) const;

    void renderBoundingBoxAndAnchorPoint(
        Falcor::RenderContext* pRenderContext,
        const Falcor::ref<Falcor::Fbo>& pTargetFbo,
        const Falcor::AABB& boundingBox,
        const SceneObject* pSceneObject,
        bool renderAnchorPoint
    ) const;

    void renderUI(Falcor::Gui::Widgets& w);

private:
    void onModuleRegistered(const std::shared_ptr<RenderModuleBase>& pModule) override;
    bool isObjectRendered(const std::shared_ptr<SceneObject>& pSceneObject) const;

    std::vector<std::shared_ptr<SceneObject>> mOrderedObjects;

    Falcor::Gui::DropdownList bRenderers;
    std::unordered_map<Falcor::uint, ModuleId> bRendererForIndex;
    std::unordered_map<ModuleId, Falcor::uint> bIndexForRenderer;

    Falcor::ref<GraphicsProgramWrapper> mpBoundingBoxRenderProgram, mpAnchorPointRenderProgram;
};


template<typename RendererT, typename RenderableT>
void RenderManager::preRenderObject(
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pTargetFbo,
    RenderableT& renderable
) const {
    static_assert(std::is_base_of_v<RenderModule<RenderableT>, RendererT>,
                  "RenderManager: Renderer for a renderable must be a RenderModule, specified for that object");

    if (const auto pRenderer = getMaybeContained<RendererT>())
    {
        pRenderer->preRenderObject(pRenderContext, pTargetFbo, renderable);
        return;
    }

    Falcor::logWarningOnce("Could not find renderer for object of type: '{}'", cereal::util::demangle(typeid(renderable).name()));
}


template<typename RendererT, typename RenderableT>
void RenderManager::renderObject(
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pTargetFbo,
    RenderableT& renderable
) const {
    static_assert(std::is_base_of_v<RenderModule<RenderableT>, RendererT>,
                  "RenderManager: Renderer for a renderable must be a RenderModule, specified for that object");

    if (const auto pRenderer = getMaybeContained<RendererT>())
    {
        pRenderer->renderObject(pRenderContext, pTargetFbo, renderable);
        return;
    }

    Falcor::logWarningOnce("Could not find renderer for object of type: '{}'", cereal::util::demangle(typeid(renderable).name()));
}


template<typename RendererT, typename RenderableT>
void RenderManager::postRenderObject(
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pTargetFbo,
    RenderableT& renderable
) const {
    static_assert(std::is_base_of_v<RenderModule<RenderableT>, RendererT>,
                  "RenderManager: Renderer for a renderable must be a RenderModule, specified for that object");

    if (const auto pRenderer = getMaybeContained<RendererT>())
    {
        pRenderer->postRenderObject(pRenderContext, pTargetFbo, renderable);
        return;
    }

    Falcor::logWarningOnce("Could not find renderer for object of type: '{}'", cereal::util::demangle(typeid(renderable).name()));
}

} // namespace GraphEx::Core



