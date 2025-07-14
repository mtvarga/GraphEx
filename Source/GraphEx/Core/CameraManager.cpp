#include "CameraManager.h"

#include "../API/EventManager.h"
#include "CoreEvents.h"


using namespace GraphEx;
using namespace GraphEx::Core;


const std::unordered_map<TypeId, std::string> CAMERA_CONTROLLER_NAMES = {
    { typeid(Falcor::FirstPersonCameraController), "First Person" },
    { typeid(Falcor::SixDoFCameraController), "First Person (6 DoF)" },
};


CameraManager::CameraManager(ModuleContainerBase* pContainer)
    : Module(pContainer)
{
    EventManager::get().registerEventHandler<KeyboardEvent>([this](const Falcor::KeyboardEvent& keyEvent) {
        return onKeyEvent(keyEvent);
    });

    EventManager::get().registerEventHandler<MouseEvent>([this](const Falcor::MouseEvent& mouseEvent) {
        return onMouseEvent(mouseEvent);
    });

    EventManager::get().registerEventHandler<GamepadEvent>([this](const Falcor::GamepadState& gamepadState) {
        return onGamepadEvent(gamepadState);
    });
}


void CameraManager::init(Falcor::RenderContext* pRenderContext)
{
    const auto pDefaultCamera = Falcor::Camera::create(getNextCameraName());
    pDefaultCamera->setUpVector({0.0f, 1.0f, 0.0f});
    pDefaultCamera->setPosition({2.0f, 2.0f, 2.0f});
    pDefaultCamera->setTarget({0.0f, 0.0f, 0.0f});
    pDefaultCamera->setNearPlane(0.001f);

    mpState->cameras.clear();
    addCamera(pDefaultCamera);
    switchCamera(0);

    bCameraControllerDropdownList.clear();
    std::transform(
        mCameraControllers.begin(),
        mCameraControllers.end(),
        std::back_inserter(bCameraControllerDropdownList),
        [this](const auto& pCameraController)
        {
            return Falcor::Gui::DropdownValue{
                static_cast<uint32_t>(bCameraControllerDropdownList.size()), CAMERA_CONTROLLER_NAMES.at(typeid(*pCameraController))
            };
        }
    );

    bActiveCameraControllerIndex = 0;
}


void CameraManager::update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo)
{
    getActiveCameraController()->update();
    getActiveCamera()->beginFrame();
}


void CameraManager::renderUI(Falcor::Gui::Widgets& w)
{
    constexpr auto CAMERA_CONTROLLER_DROPDOWN_LABEL = "Camera Controller";

    const auto& style             = ImGui::GetStyle();
    const auto  itemInnerSpacingH = style.ItemInnerSpacing.x;
    const auto  itemSpacingH      = style.ItemSpacing.x;

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(CAMERA_CONTROLLER_DROPDOWN_LABEL).x - itemInnerSpacingH);
    w.dropdown(CAMERA_CONTROLLER_DROPDOWN_LABEL, bCameraControllerDropdownList, bActiveCameraControllerIndex);

    if (ImGui::TreeNodeEx("Cameras", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto       shouldRemoveCamera       = false;
        auto       cameraIndexToRemove      = 0u;
        const auto shouldRenderRemoveButton = mpState->cameras.size() > 1;

        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });

        for (uint32_t i = 0; i < mpState->cameras.size(); ++i)
        {
            const auto& pCamera    = mpState->cameras.at(i);
            const auto  isActive   = i == mpState->activeCamera;
            const auto  buttonSize = ImVec2 {
                ImGui::GetContentRegionAvail().x - (shouldRenderRemoveButton ? ImGui::GetFrameHeight() + itemSpacingH : 0.0f),
                0.0f
            };

            if (!isActive)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
            }

            bCameraButtons.at(i).render((pCamera->getName() + "##" + std::to_string(i)).c_str(), buttonSize,
                [this, i]() {
                    switchCamera(i);
                },
                [this, i](const char* newName) {
                    mpState->cameras.at(i)->setName(newName);
                }
            );

            if (!isActive)
            {
                ImGui::PopStyleColor();
            }

            if (shouldRenderRemoveButton)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.5f, 0.5f });
                ImGui::SameLine();

                if (ImGui::Button(("X##" + std::to_string(i)).c_str(), ImVec2{ ImGui::GetFrameHeight(), ImGui::GetFrameHeight() }))
                {
                    shouldRemoveCamera = true;
                    cameraIndexToRemove = i;
                }

                ImGui::PopStyleVar();
            }
        }

        ImGui::PopStyleVar();

        if (shouldRemoveCamera)
        {
            removeCamera(cameraIndexToRemove);
        }

        ImGui::Dummy(ImVec2{ 0.0f, ImGui::GetFrameHeight() / -2.0f });

        if (w.button("Add Camera"))
        {
            const auto pNewCamera = Falcor::make_ref<Falcor::Camera>(*getActiveCamera());
            pNewCamera->setName(getNextCameraName());
            addCamera(pNewCamera);
        }

        w.checkbox("Make New Camera Active", mpState->makeNewCameraActive, true);

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Active Camera Properties", ImGuiTreeNodeFlags_DefaultOpen))
    {
        getActiveCamera()->renderUI(w);
        ImGui::TreePop();
    }
}


void CameraManager::cleanup() {}


ModuleId CameraManager::getModuleId() const
{
    return "GraphEx.CameraManager";
}


void CameraManager::setProgramVars(const Falcor::ShaderVar& var) const
{
    getActiveCamera()->bindShaderData(var);
}


void CameraManager::setState(std::shared_ptr<CameraManagerState> pState)
{
    HasSerializableState::setState(std::move(pState));

    if (mpState->activeCamera > mpState->cameras.size())
    {
        mpState->activeCamera = 0;
    }

    switchCamera(mpState->activeCamera);
    mCameraCounter = mpState->cameras.size();
    bCameraButtons.resize(mpState->cameras.size(), UIHelpers::DynamicButton{ });
}


Falcor::ref<Falcor::Camera> CameraManager::getActiveCamera() const
{
    if (mpState->activeCamera >= mpState->cameras.size())
    {
        return nullptr;
    }

    return mpState->cameras.at(mpState->activeCamera);
}


std::shared_ptr<Falcor::CameraController> CameraManager::getActiveCameraController() const
{
    if (bActiveCameraControllerIndex >= mCameraControllers.size())
    {
        return nullptr;
    }

    return mCameraControllers.at(bActiveCameraControllerIndex);
}


uint32_t CameraManager::addCamera(const Falcor::ref<Falcor::Camera>& pCamera)
{
    mpState->cameras.push_back(pCamera);
    bCameraButtons.emplace_back();

    const auto result = mpState->cameras.size() - 1;

    if (mpState->makeNewCameraActive)
    {
        switchCamera(result);
    }

    return result;
}


void CameraManager::switchCamera(const uint32_t index)
{
    if (index >= mpState->cameras.size())
    {
        return;
    }

    mpState->activeCamera = index;

    mCameraControllers.clear();
    mCameraControllers.emplace_back(std::make_shared<Falcor::FirstPersonCameraController>(mpState->cameras[mpState->activeCamera]));
    mCameraControllers.emplace_back(std::make_shared<Falcor::SixDoFCameraController>(mpState->cameras[mpState->activeCamera]));
}


void CameraManager::removeCamera(const uint32_t index)
{
    if (index >= mpState->cameras.size())
    {
        return;
    }

    mpState->cameras.erase(mpState->cameras.begin() + index);
    bCameraButtons.pop_back();

    if (mpState->activeCamera >= mpState->cameras.size())
    {
        switchCamera(0);
    }
}


bool CameraManager::onKeyEvent(const Falcor::KeyboardEvent& keyEvent) const
{
    return getActiveCameraController() ? getActiveCameraController()->onKeyEvent(keyEvent) : false;
}


bool CameraManager::onMouseEvent(const Falcor::MouseEvent& mouseEvent) const
{
    return getActiveCameraController() ? getActiveCameraController()->onMouseEvent(mouseEvent) : false;
}


bool CameraManager::onGamepadEvent(const Falcor::GamepadState& gamepadState) const
{
    return getActiveCameraController() ? getActiveCameraController()->onGamepadState(gamepadState) : false;
}


std::string CameraManager::getNextCameraName()
{
    return "Camera " + std::to_string(++mCameraCounter);
}
