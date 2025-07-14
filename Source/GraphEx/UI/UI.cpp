#include "UI.h"

#include "../Core/SceneManager.h"
#include "../Core/CameraManager.h"
#include "../Core/RenderManager.h"

#include "UIHelpers.h"


using namespace GraphEx;


UI::UI(Application* app)
    : mpApp(app)
    , mWindowSize({ 0, 0 }) {}


void UI::render(Falcor::Gui* pGui)
{
    renderMainMenuBar(pGui);

    if (!mpApp->containsAll<Core::CameraManager, Core::SceneManager, Core::RenderManager>())
    {
        FALCOR_THROW(
            "Failed to render UI: at least one of the following core modules is missing: "
            "CameraManager, SceneManager, RenderManager"
        );
    }

    const auto [ cameraManager, sceneManager, renderManager ] =
        mpApp->getsContained<Core::CameraManager, Core::SceneManager, Core::RenderManager>();

    renderSceneCameraManagerWindow(pGui, sceneManager, cameraManager);
    renderRenderManagerWindow(pGui, renderManager);
}


void UI::renderMainMenuBar(Falcor::Gui* pGui)
{
    // Draw the main menu bar
    {
        auto mainMenu = Falcor::Gui::MainMenu{ pGui };

        // Draw "File" menu
        {
            auto fileMenu = mainMenu.dropdown("File");

            if (!mpApp->getProjectFilePath().empty())
            {
                if (fileMenu.item("Save Project"))
                {
                    mpApp->saveProject();
                }
            }

            if (fileMenu.item("Save Project As..."))
            {
                if (std::filesystem::path path;
                    Falcor::saveFileDialog({ { "gxproj", "GraphEx Project" } }, path))
                {
                    mpApp->saveProjectAs(path);
                }
            }

            if (fileMenu.item("Open Project..."))
            {
                if (std::filesystem::path path;
                    Falcor::openFileDialog({ { "gxproj", "GraphEx Project" } }, path))
                {
                    mpApp->loadProject(path);
                }
            }

            fileMenu.separator();

            if (fileMenu.item("Exit"))
            {
                mpApp->shutdown();
            }
        }

        // Draw View Menu
        {
            auto viewMenu = mainMenu.dropdown("View");
            viewMenu.item("Lock Windows", mWindowsLocked);
        }
    }
}


void UI::renderSceneCameraManagerWindow(Falcor::Gui* pGui, Core::SceneManager& sceneManager, Core::CameraManager& cameraManager) const
{
    constexpr auto UNLOCKED_FLAGS = Falcor::Gui::WindowFlags::AllowMove;
    constexpr auto LOCKED_FLAGS = Falcor::Gui::WindowFlags::NoResize;

    auto w = Falcor::Gui::Window {
        pGui,
        "##SceneCameraManagerWindow",
        { 0, 0 },
        { 0, 0 },
        mWindowsLocked ? LOCKED_FLAGS : UNLOCKED_FLAGS
    };

    if (mWindowsLocked)
    {
        const auto [ x, y ] = getSceneCameraManagerWindowPos();
        const auto [ width, height ] = getSceneCameraManagerWindowSize();

        w.windowPos(x, y);
        w.windowSize(width, height);
    }

    if (ImGui::BeginTabBar("##SceneCameraManagerWindowTabs"))
    {
        if (ImGui::BeginTabItem("Scene"))
        {
            sceneManager.renderUI(w);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Cameras"))
        {
            cameraManager.renderUI(w);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}


void UI::renderRenderManagerWindow(Falcor::Gui* pGui, Core::RenderManager& renderManager) const
{
    constexpr auto UNLOCKED_FLAGS = Falcor::Gui::WindowFlags::AllowMove;
    constexpr auto LOCKED_FLAGS = Falcor::Gui::WindowFlags::NoResize;

    auto w = Falcor::Gui::Window {
        pGui,
        "##RenderManagerWindow",
        { 0, 0 },
        { 0, 0 },
        mWindowsLocked ? LOCKED_FLAGS : UNLOCKED_FLAGS
    };

    if (mWindowsLocked)
    {
        const auto [ x, y ] = getRenderManagerWindowPos();
        const auto [ width, height ] = getRenderManagerWindowSize();

        w.windowPos(x, y);
        w.windowSize(width, height);
    }

    if (ImGui::BeginTabBar("##RenderManagerWindowTabs"))
    {
        if (ImGui::BeginTabItem("Rendering"))
        {
            renderManager.renderUI(w);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}


std::pair<uint32_t, uint32_t> UI::getSceneCameraManagerWindowPos() const
{
    return UIHelpers::getLeftWindowStart();
}


std::pair<uint32_t, uint32_t> UI::getSceneCameraManagerWindowSize() const
{
    auto area = UIHelpers::getLeftWindowArea(*this);
    area.second *= 0.3333f; // 1/3 of the window height
    return area;
}


std::pair<uint32_t, uint32_t> UI::getRenderManagerWindowPos() const
{
    auto pos = getSceneCameraManagerWindowPos();
    pos.second += getSceneCameraManagerWindowSize().second + ImGui::GetFrameHeight();
    return pos;
}


std::pair<uint32_t, uint32_t> UI::getRenderManagerWindowSize() const
{
    auto area = UIHelpers::getLeftWindowArea(*this);
    area.second *= 2.0f / 3.0f; // 2/3 of the window height
    return area;
}


bool UI::areWindowsLocked() const
{
    return mWindowsLocked;
}
