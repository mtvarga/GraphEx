#include "RenderManager.h"

#include "../API/EventManager.h"
#include "CoreEvents.h"


using namespace GraphEx;
using namespace GraphEx::Core;


RenderManager::RenderManager(ModuleContainerBase* pContainer)
    : ContainerModule(pContainer), Requires(pContainer)
{
    // Register render-related core events
    EventManager::get().registerEvent<EventRenderWillBegin>();
    EventManager::get().registerEvent<EventRenderBegan>();
    EventManager::get().registerEvent<EventRenderWillEnd>();
    EventManager::get().registerEvent<EventRenderEnded>();

    // Subscribe to scene-related events
    EventManager::get().registerEventHandler<EventSceneObjectAdded>([this](const std::shared_ptr<SceneObject>& pSceneObject) {
        onSceneObjectAdded(pSceneObject);
    });

    EventManager::get().registerEventHandler<EventSceneObjectRemoved>([this](const std::shared_ptr<SceneObject>& pSceneObject) {
        onSceneObjectRemoved(pSceneObject);
    });
}


void RenderManager::init(Falcor::RenderContext* pRenderContext)
{
    for (const auto& pModule : getAllModules())
    {
        pModule->init(pRenderContext);
    }

    mpBoundingBoxRenderProgram = GraphicsProgramWrapper::create(
        pRenderContext->getDevice(), Common::getBoundingBoxVertexShaderPath(), Common::getBoundingBoxPixelShaderPath()
    );

    mpBoundingBoxRenderProgram->setVao(Falcor::Vao::create(Falcor::Vao::Topology::LineList));

    mpAnchorPointRenderProgram = GraphicsProgramWrapper::create(
        pRenderContext->getDevice(), Common::getAnchorPointVertexShaderPath(), Common::getAnchorPointPixelShaderPath()
    );

    mpAnchorPointRenderProgram->setVao(Falcor::Vao::create(Falcor::Vao::Topology::TriangleStrip));
    mpAnchorPointRenderProgram->getState()->setDepthStencilState(
        Falcor::DepthStencilState::create(Falcor::DepthStencilState::Desc().setDepthEnabled(false).setDepthWriteMask(false))
    );
}


void RenderManager::update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), mpState->backgroundColor, 1.0f, 0, Falcor::FboAttachmentType::All);

    EventManager::get().dispatchEvent<EventRenderWillBegin>();

    for (const auto& pSceneObject : mOrderedObjects)
    {
        pSceneObject->preRender(*this, pRenderContext, pTargetFbo);
    }

    EventManager::get().dispatchEvent<EventRenderBegan>();

    for (const auto& pSceneObject : mOrderedObjects)
    {
        pSceneObject->render(*this, pRenderContext, pTargetFbo);
    }

    EventManager::get().dispatchEvent<EventRenderWillEnd>();

    for (const auto& pSceneObject : mOrderedObjects)
    {
        pSceneObject->postRender(*this, pRenderContext, pTargetFbo);
    }

    EventManager::get().dispatchEvent<EventRenderEnded>();
}


void RenderManager::cleanup()
{
    for (const auto& pModule : getAllModules())
    {
        pModule->cleanup();
    }
}


ModuleId RenderManager::getModuleId() const
{
    return "GraphEx.RenderManager";
}


void RenderManager::setState(std::shared_ptr<RenderManagerState> pState)
{
    HasSerializableState::setState(std::move(pState));

    // Clear the current scene objects
    mOrderedObjects.clear();

    for (const auto& pSceneObjectState : mpState->orderedObjectStates)
    {
        if (pSceneObjectState.success() && pSceneObjectState.get()->isValid())
        {
            mOrderedObjects.push_back(pSceneObjectState.get());
        }
    }
}


const std::shared_ptr<RenderManagerState>& RenderManager::getState() const
{
    const auto& result = HasSerializableState::getState();
    result->orderedObjectStates.clear();
    result->orderedObjectStates.reserve(mOrderedObjects.size());

    for (const auto& pSceneObject : mOrderedObjects)
    {
        result->orderedObjectStates.emplace_back(pSceneObject);
    }

    return result;
}



void RenderManager::onSceneObjectAdded(const std::shared_ptr<SceneObject>& pSceneObject)
{
    if (isObjectRendered(pSceneObject))
    {
        FALCOR_THROW("RenderManager: Attempted to add same SceneObject twice.");
    }

    mOrderedObjects.push_back(pSceneObject);
}


void RenderManager::onSceneObjectRemoved(const std::shared_ptr<SceneObject>& pSceneObject)
{
    if (!isObjectRendered(pSceneObject))
    {
        return;
    }

    mOrderedObjects.erase(std::remove(mOrderedObjects.begin(), mOrderedObjects.end(), pSceneObject),
        mOrderedObjects.end());
}


Falcor::ref<const Falcor::Camera> RenderManager::getActiveCamera() const
{
    return getRequired<CameraManager>().getActiveCamera();
}


void RenderManager::setBuiltinRenderVars(const Falcor::ShaderVar& rootVar, const SceneObject* pSceneObject) const
{
    if (!rootVar.hasMember("graphEx"))
    {
        FALCOR_THROW("Attempted to set builtin render vars on a ShaderVar that does not have the appropriate 'graphEx' member.");
    }

    const auto graphEx = rootVar["graphEx"];

    ProgramVarProvider::trySetProgramVarsFor(graphEx, "_activeCamera", getRequired<CameraManager>());
    ProgramVarProvider::trySetProgramVarsFor(graphEx, "_scene", getRequired<SceneManager>());

    if (pSceneObject)
    {
        ProgramVarProvider::trySetProgramVarsFor(graphEx, "_model", *pSceneObject);
    }
}


void RenderManager::renderBoundingBoxAndAnchorPoint(
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pTargetFbo,
    const Falcor::AABB& boundingBox,
    const SceneObject* pSceneObject,
    const bool renderAnchorPoint
) const
{
    if (pSceneObject && renderAnchorPoint)
    {
        auto& anchorPointRenderProgram = *mpAnchorPointRenderProgram;

        setBuiltinRenderVars(anchorPointRenderProgram.getRootVar(), pSceneObject);
        anchorPointRenderProgram["VScb"]["anchorPoint"] = pSceneObject->getTransform().getAnchorPoint();
        anchorPointRenderProgram["VScb"]["screenScaling"] = Falcor::getDisplayScaleFactor();
        anchorPointRenderProgram["VScb"]["screenSize"] = Falcor::uint2{ pTargetFbo->getWidth(), pTargetFbo->getHeight() };
        anchorPointRenderProgram["VScb"]["pointSize"] = 10.0f;
        anchorPointRenderProgram["PScb"]["color"] = Falcor::float3{ 1.0f, 0.0f, 0.0f };
        anchorPointRenderProgram.draw(pRenderContext, pTargetFbo, 6);
    }

    auto& boundingBoxRenderProgram = *mpBoundingBoxRenderProgram;

    setBuiltinRenderVars(boundingBoxRenderProgram.getRootVar(), pSceneObject);
    boundingBoxRenderProgram["VScb"]["modelTrans"] = boundingBox.minPoint;
    boundingBoxRenderProgram["VScb"]["modelScale"] = boundingBox.extent();
    boundingBoxRenderProgram["VScb"]["worldMatrix"] =
        pSceneObject ? pSceneObject->getTransform().getWorldMatrix() : Falcor::float4x4::identity();

    boundingBoxRenderProgram["PScb"]["color"] = Falcor::float3{0.5f, 0.0f, 1.0f};
    boundingBoxRenderProgram.draw(pRenderContext, pTargetFbo, 24);
}


void RenderManager::renderUI(Falcor::Gui::Widgets& w)
{
    // Render global settings GUI
    {
        if (w.checkbox("VSync", mpState->vSync))
        {
            mpContainer->toggleVsync(mpState->vSync);
        }

        ImGui::PushItemWidth(200);
        w.rgbaColor("Background Color", mpState->backgroundColor);
        ImGui::PopItemWidth();
    }

    // Render renderer selector and settings
    if (!bRenderers.empty())
    {
        if (mpState->selectedRendererIndex >= bRenderers.size())
        {
            mpState->selectedRendererIndex = 0;
        }

        w.separator();
        w.text("View Settings For Renderer");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        w.dropdown("##renderer_selector", bRenderers, mpState->selectedRendererIndex);
        w.separator();
        getContained<RenderModuleBase>(bRendererForIndex.at(mpState->selectedRendererIndex)).renderGlobalSettingsGui(w);
    }

    // Render object list group
    if (w.group("Objects", true))
    {
        unsigned int selectedObjectIndex = -1;

        if (!mOrderedObjects.empty())
        {
            constexpr auto MOVE_UP_BUTTON_LABEL = "Move Up##";
            constexpr auto MOVE_DOWN_BUTTON_LABEL = "Move Down##";

            const auto& style = ImGui::GetStyle();
            const auto  moveUpButtonWidth = 2.0f * style.FramePadding.x + ImGui::CalcTextSize(MOVE_UP_BUTTON_LABEL, nullptr, true).x;
            const auto  moveDownButtonWidth = 2.0f * style.FramePadding.x + ImGui::CalcTextSize(MOVE_DOWN_BUTTON_LABEL, nullptr, true).x;
            const auto  doubleButtonWidth = moveUpButtonWidth + style.ItemSpacing.x + moveDownButtonWidth;

            auto shouldSwap = false;
            auto swapIndex = 0u;
            auto swapDir = 1;

            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });

            for (uint32_t i = 0; i < mOrderedObjects.size(); ++i)
            {
                const auto& pSceneObject = mOrderedObjects.at(i);
                const auto  isActive   = pSceneObject->isSelected();
                const auto  isFirst = i == 0;
                const auto  isLast = i == mOrderedObjects.size() - 1;
                const auto  isOnly = isFirst && isLast;
                const auto  buttonWidth =
                    ImGui::GetContentRegionAvail().x -
                        (isOnly ? 0.0f : (isFirst ? moveDownButtonWidth : isLast ? moveUpButtonWidth : doubleButtonWidth) + style.ItemSpacing.x);

                if (!isActive)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
                }
                else
                {
                    selectedObjectIndex = i;
                }

                ImGui::Button((pSceneObject->getHumanReadableName() + "##" + std::to_string(i)).c_str(), ImVec2 { buttonWidth, 0.0f });

                if (!isActive)
                {
                    ImGui::PopStyleColor();
                }

                if (!isFirst)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.5f, 0.5f });

                    if (w.button((MOVE_UP_BUTTON_LABEL + std::to_string(i)).c_str(), true))
                    {
                        shouldSwap = true;
                        swapIndex = i;
                        swapDir = -1;
                    }

                    ImGui::PopStyleVar();
                }

                if (!isLast)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.5f, 0.5f });

                    if (w.button((MOVE_DOWN_BUTTON_LABEL + std::to_string(i)).c_str(), true))
                    {
                        shouldSwap = true;
                        swapIndex = i;
                        swapDir = 1;
                    }

                    ImGui::PopStyleVar();
                }
            }

            if (shouldSwap)
            {
                std::iter_swap(mOrderedObjects.begin() + swapIndex, mOrderedObjects.begin() + swapIndex + swapDir);
            }

            ImGui::PopStyleVar();

            w.separator();
        }
        else
        {
            w.text("There are no objects to render.");
        }

        // Render properties for selected object
        if (ImGui::TreeNodeEx("Selected Object Render Properties", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (selectedObjectIndex < mOrderedObjects.size())
            {
                if (const auto& selectedObject = mOrderedObjects[selectedObjectIndex];
                    selectedObject->hasSettings())
                {
                    selectedObject->renderSettingsUI(w);
                }
                else
                {
                    w.text("This object has no adjustable settings.");
                }
            }
            else
            {
                w.text("There is no object selected.");
            }

            ImGui::TreePop();
        }
    }
}


void RenderManager::onModuleRegistered(const std::shared_ptr<RenderModuleBase>& pModule)
{
    ModuleContainer::onModuleRegistered(pModule);

    bRenderers.clear();
    bRendererForIndex.clear();
    bIndexForRenderer.clear();

    const auto modules = getAllModules();
    bRenderers.reserve(modules.size());

    for (Falcor::uint i = 0; i < modules.size(); ++i)
    {
        if (const auto& currentModule = modules.at(i);
            currentModule->hasGlobalSettings())
        {
            bRenderers.emplace_back(Falcor::Gui::DropdownValue{ i, modules.at(i)->getHumanReadableName() });
            bRendererForIndex.emplace(i, modules.at(i)->getModuleId());
            bIndexForRenderer.emplace(modules.at(i)->getModuleId(), i);
        }
    }
}


bool RenderManager::isObjectRendered(const std::shared_ptr<SceneObject>& pSceneObject) const
{
    return std::find(mOrderedObjects.cbegin(), mOrderedObjects.cend(), pSceneObject) != mOrderedObjects.cend();
}
