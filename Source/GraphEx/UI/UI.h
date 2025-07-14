#pragma once

#include "../Utils/Standard.h"


namespace GraphEx
{

struct GRAPHEX_EXPORTABLE Application;

namespace Core
{

struct GRAPHEX_EXPORTABLE SceneManager;
struct GRAPHEX_EXPORTABLE CameraManager;
struct GRAPHEX_EXPORTABLE RenderManager;

} // namespace GraphEx::Core


struct GRAPHEX_EXPORTABLE UI
{
    explicit UI(Application* app);

    void render(Falcor::Gui* pGui);

private:
    void renderMainMenuBar(Falcor::Gui* pGui);
    void renderSceneCameraManagerWindow(Falcor::Gui* pGui, Core::SceneManager& sceneManager, Core::CameraManager& cameraManager) const;
    void renderRenderManagerWindow(Falcor::Gui* pGui, Core::RenderManager& renderManager) const;

    std::pair<uint32_t, uint32_t> getSceneCameraManagerWindowPos() const;
    std::pair<uint32_t, uint32_t> getSceneCameraManagerWindowSize() const;
    std::pair<uint32_t, uint32_t> getRenderManagerWindowPos() const;
    std::pair<uint32_t, uint32_t> getRenderManagerWindowSize() const;

    Application* mpApp;
    Falcor::uint2 mWindowSize;
    bool mWindowsLocked = true;

public:
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(WindowSize, mWindowSize)
    bool areWindowsLocked() const;
};

} // namespace GraphEx