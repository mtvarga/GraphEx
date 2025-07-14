#include "SceneManager.h"

#include "../API/EventManager.h"
#include "CoreEvents.h"


using namespace GraphEx::Core;


SceneManager::SceneManager(ModuleContainerBase* pContainer)
    : Module(pContainer)
{
    EventManager::get().registerEvent<EventSceneObjectAdded>();
    EventManager::get().registerEvent<EventSceneObjectRemoved>();

    mpState->pGlobalLight = std::make_shared<Light>(Light::Type::Directional);
}


void SceneManager::renderUI(Falcor::Gui::Widgets& w)
{
    const auto style = ImGui::GetStyle();
    const auto itemSpacingH = style.ItemSpacing.x;

    // Render global light properties
    if (ImGui::TreeNodeEx("Global Light Properties"))
    {
        mpState->pGlobalLight->renderGui(w);
        ImGui::TreePop();
    }

    // Render object list
    if (ImGui::TreeNodeEx("Objects", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (!mSceneObjects.empty())
        {
            auto shouldRemoveObject = false;
            std::shared_ptr<SceneObject> objectToRemove = nullptr;

            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });

            for (uint32_t i = 0; i < mSceneObjects.size(); ++i)
            {
                const auto& pSceneObject = mSceneObjects.at(i);
                const auto  isActive = pSceneObject->isSelected();

                const auto buttonSize = ImVec2 {
                    ImGui::GetContentRegionAvail().x - ImGui::GetFrameHeight() - itemSpacingH,
                    0.0f
                };

                if (!isActive)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
                }

                bSceneObjectButtons.at(i).render(pSceneObject->getHumanReadableName() + "##" + std::to_string(i), buttonSize,
                    [this, i] {
                        selectSceneObject(i);
                    },
                    [&pSceneObject](const char* newName) {
                        pSceneObject->setHumanReadableName(newName);
                    }
                );

                if (!isActive)
                {
                    ImGui::PopStyleColor();
                }

                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.5f, 0.5f });
                ImGui::SameLine();

                if (ImGui::Button(("X##" + std::to_string(i)).c_str(), ImVec2{ ImGui::GetFrameHeight(), ImGui::GetFrameHeight() }))
                {
                    shouldRemoveObject = true;
                    objectToRemove = pSceneObject;
                }

                ImGui::PopStyleVar();
            }

            ImGui::PopStyleVar();

            if (shouldRemoveObject)
            {
                removeSceneObject(objectToRemove);
            }
        }
        else
        {
            w.text("There are no objects in the current scene.");
        }

        ImGui::TreePop();
    }

    // Render properties for selected object
    if (ImGui::TreeNodeEx("Selected Object Properties", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (mpState->selectedSceneObjectIndex < mSceneObjects.size())
        {
            mSceneObjects.at(mpState->selectedSceneObjectIndex)->renderGui(w);
        }
        else
        {
            w.text("No object selected.");
        }

        ImGui::TreePop();
    }
}


GraphEx::ModuleId SceneManager::getModuleId() const
{
    return "GraphEx.SceneManager";
}


void SceneManager::addSceneObject(const std::shared_ptr<SceneObject>& pSceneObject)
{
    mSceneObjects.push_back(pSceneObject);
    bSceneObjectButtons.emplace_back();

    selectSceneObject(mSceneObjects.size() - 1);

    EventManager::get().enqueueEvent<EventSceneObjectAdded>(pSceneObject);
}


void SceneManager::removeSceneObject(const std::shared_ptr<SceneObject>& pSceneObject)
{
    mSceneObjects.erase(
        std::remove_if(
            mSceneObjects.begin(),
            mSceneObjects.end(),
            [&pSceneObject](const auto& pSceneObjectInList) { return pSceneObjectInList == pSceneObject; }
        ),
        mSceneObjects.end()
    );

    if (pSceneObject->isSelected())
    {
        pSceneObject->setSelected(false);
        selectSceneObject(0);
    }

    bSceneObjectButtons.pop_back();
    EventManager::get().enqueueEvent<EventSceneObjectRemoved>(pSceneObject);
}


void SceneManager::setProgramVars(const Falcor::ShaderVar& var) const
{
    trySetProgramVarsFor(var, "_globalLight", *mpState->pGlobalLight);
}


void SceneManager::setState(std::shared_ptr<SceneManagerState> pState)
{
    bool foundIncompatibleSceneObject = false, foundInvalidSceneObject = false;
    HasSerializableState::setState(std::move(pState));

    // Clear the current scene objects
    mSceneObjects.clear();

    for (const auto& pSceneObjectState : mpState->sceneObjectStates)
    {
        if (!pSceneObjectState.success())
        {
            foundIncompatibleSceneObject = true;
            continue;
        }

        if (!pSceneObjectState.get()->isValid())
        {
            foundInvalidSceneObject = true;
            continue;
        }

        mSceneObjects.push_back(pSceneObjectState.get());
    }

    if (foundIncompatibleSceneObject)
    {
        msgBox("Warning", "Some scene objects from the project file were not loaded because they were incompatible with the current "
               "version of the program. Check the log for details.", Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Warning);
    }

    if (!foundIncompatibleSceneObject && foundInvalidSceneObject)
    {
        // We skip logging this message if incompatible objects were found, that message already suggests that the project file was bad
        msgBox("Warning", "Some scene objects from the project file were not loaded because they were not valid.",
               Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Warning);
    }

    selectSceneObject(mpState->selectedSceneObjectIndex);
    bSceneObjectButtons.resize(mSceneObjects.size(), UIHelpers::DynamicButton{ });
}


const std::shared_ptr<SceneManagerState>& SceneManager::getState() const
{
    const auto& result = HasSerializableState::getState();
    result->sceneObjectStates.clear();
    result->sceneObjectStates.reserve(mSceneObjects.size());

    for (const auto& pSceneObject : mSceneObjects)
    {
        result->sceneObjectStates.emplace_back(pSceneObject);
    }

    return result;
}


void SceneManager::selectSceneObject(const uint32_t index) const
{
    if (index >= mSceneObjects.size())
    {
        return;
    }

    if (mpState->selectedSceneObjectIndex < mSceneObjects.size())
    {
        mSceneObjects.at(mpState->selectedSceneObjectIndex)->setSelected(false);
    }

    mSceneObjects.at(index)->setSelected(true);
    mpState->selectedSceneObjectIndex = index;
}


void SceneManager::init(Falcor::RenderContext*) {}
void SceneManager::update(Falcor::RenderContext*, const Falcor::ref<Falcor::Fbo>&) {}
void SceneManager::cleanup() {}
