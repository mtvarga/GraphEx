#pragma once

#include "../API/Module.h"
#include "../UI/UIHelpers.h"


namespace GraphEx::Core
{

struct GRAPHEX_EXPORTABLE CameraManagerState final : ModuleState
{
    bool makeNewCameraActive = false;
    std::vector<Falcor::ref<Falcor::Camera>> cameras;
    uint32_t activeCamera = 0;

    template<typename Archive>
    void serialize(Archive& ar);
};


template<typename Archive>
void CameraManagerState::serialize(Archive& ar)
{
    ar(GRAPHEX_SERIALIZE_WITH_NAME(makeNewCameraActive));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(activeCamera));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(cameras));
}


struct GRAPHEX_EXPORTABLE CameraManager final : Module, ProgramVarProvider, HasSerializableState<CameraManagerState>
{
    explicit CameraManager(ModuleContainerBase* pContainer);

    void init(Falcor::RenderContext* pRenderContext) override;
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override;
    void cleanup() override;

    ModuleId getModuleId() const override;

    void setProgramVars(const Falcor::ShaderVar& var) const override;

    void setState(std::shared_ptr<CameraManagerState> pState) override;

    Falcor::ref<Falcor::Camera> getActiveCamera() const;
    std::shared_ptr<Falcor::CameraController> getActiveCameraController() const;

    uint32_t addCamera(const Falcor::ref<Falcor::Camera>& pCamera);
    void switchCamera(uint32_t index);
    void removeCamera(uint32_t index);

    void renderUI(Falcor::Gui::Widgets& w);

private:
    bool onKeyEvent(const Falcor::KeyboardEvent& keyEvent) const;
    bool onMouseEvent(const Falcor::MouseEvent& mouseEvent) const;
    bool onGamepadEvent(const Falcor::GamepadState& gamepadState) const;

    std::string getNextCameraName();

    std::vector<std::shared_ptr<Falcor::CameraController>> mCameraControllers;

    std::vector<UIHelpers::DynamicButton> bCameraButtons;
    Falcor::Gui::DropdownList bCameraControllerDropdownList;
    uint32_t bActiveCameraControllerIndex = 0;

    uint32_t mCameraCounter = 0;

public:
    DEFAULT_CONST_GETTER_SETTER_DEFINITION(MakeNewCameraActive, mpState->makeNewCameraActive)
};


} // namespace GraphEx
