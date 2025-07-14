#pragma once

#include "../API/Module.h"
#include "../UI/UIHelpers.h"
#include "CoreTypes.h"


namespace GraphEx::Core
{

struct GRAPHEX_EXPORTABLE SceneManagerState : ModuleState
{
    std::vector<SerializedSceneObjectState> sceneObjectStates;
    std::shared_ptr<Light> pGlobalLight;
    uint32_t selectedSceneObjectIndex = 0;

    template<typename Archive>
    void serialize(Archive& ar);
};


template<typename Archive>
void SceneManagerState::serialize(Archive& ar)
{
    ar(SerializeNamed<Archive>("globalLight", pGlobalLight));
    ar(SerializeNamed<Archive>("selected", selectedSceneObjectIndex));
    ar(SerializeNamed<Archive>("objects", sceneObjectStates));
}


struct GRAPHEX_EXPORTABLE SceneManager : Module, ProgramVarProvider, HasSerializableState<SceneManagerState>
{
    explicit SceneManager(ModuleContainerBase* pContainer);

    ModuleId getModuleId() const override;

    void addSceneObject(const std::shared_ptr<SceneObject>& pSceneObject);
    void removeSceneObject(const std::shared_ptr<SceneObject>& pSceneObject);

    void setProgramVars(const Falcor::ShaderVar& var) const override;

    void setState(std::shared_ptr<SceneManagerState> pState) override;
    const std::shared_ptr<SceneManagerState>& getState() const override;

    void selectSceneObject(uint32_t index) const;

    void renderUI(Falcor::Gui::Widgets& w);

private:
    // Unused methods of Module
    void init(Falcor::RenderContext*) override;
    void update(Falcor::RenderContext*, const Falcor::ref<Falcor::Fbo>&) override;
    void cleanup() override;

    std::vector<std::shared_ptr<SceneObject>> mSceneObjects;
    std::vector<UIHelpers::DynamicButton>  bSceneObjectButtons;
};

} // namespace GraphEx::Core
