/***************************************************************************
 # Copyright (c) 2015-22, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#include "Application.h"

#include "API/EventManager.h"

#include "Core/CameraManager.h"
#include "Core/CoreEvents.h"
#include "Core/SceneManager.h"
#include "Core/RenderManager.h"


using namespace GraphEx;


Application::Application(const Falcor::SampleAppConfig& config)
    : SampleApp(config)
    , mUI(this)
{
    registerCoreEvents();
    registerCoreModules();
}


void Application::registerCoreEvents()
{
    EventManager::get().registerEvent<Core::EventFrameWillBegin>();
    EventManager::get().registerEvent<Core::EventFrameEnded>();
    EventManager::get().registerEvent<Core::KeyboardEvent>();
    EventManager::get().registerEvent<Core::MouseEvent>();
    EventManager::get().registerEvent<Core::GamepadEvent>();
}


void Application::registerCoreModules()
{
    registerModule<Core::SceneManager>();
    registerModule<Core::CameraManager>();
    registerModule<Core::RenderManager>();
}


void Application::saveProject()
{
    if (mProjectFilePath.empty() || !std::filesystem::exists(mProjectFilePath))
    {
        return;
    }

    saveProjectAs(mProjectFilePath);
}


void Application::saveProjectAs(const std::filesystem::path& filePath)
{
    if (filePath.empty())
    {
        return;
    }

    std::ostringstream oss;

    if (auto ar = Internal::SerializationManager::get().beginSave(oss))
    {
        auto error = false;

        try
        {
            ModuleRegistry::get().saveModuleStates(*ar);
        }
        catch (const std::exception& e)
        {
            Falcor::logError("Failed to save program state. See details below:\n{}", e.what());
            msgBox("Error", "Failed to save project file: an error was encountered while serializing the program's state. Check the logs "
                   "for more details.", Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
            error = true;
        }
        catch (...)
        {
            msgBox("Error", "Failed to save project file: an error was encountered while serializing the program's state.",
                   Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
            error = true;
        }

        Internal::SerializationManager::get().finish();

        if (error)
        {
            return;
        }
    }
    else
    {
        msgBox("Error", "Failed to save project file: failed to open a JSON output stream. This should not happen and is likely a bug. "
               "Check the logs for more details.", Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
        return;
    }

    std::ofstream os(filePath, std::ios::out | std::ios::trunc);

    if (!os.is_open())
    {
        msgBox("Error", "Failed to save project file: could not write to given path. Perhaps you lack permissions to write for the "
               "given path.", Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
        return;
    }

    try
    {
        os << oss.str();
        os.close();
        mProjectFilePath = filePath;
    }
    catch (...)
    {
        msgBox("Error", "Failed to save project file: could not write to file.", Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
    }
}


void Application::loadProject(const std::filesystem::path& filePath)
{
    std::ifstream is(filePath, std::ios::in);

    if (!is.is_open())
    {
        msgBox("Error", "Failed to open the provided project file.", Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
        return;
    }

    if (auto ar = Internal::SerializationManager::get().beginLoad(is))
    {
        try
        {
            ModuleRegistry::get().loadModuleStates(*ar);
            mProjectFilePath = filePath;
        }
        catch (const std::exception& e)
        {
            Falcor::logError("Failed to load program state. See details below:\n{}", e.what());
            msgBox("Error", "Failed to load project file: the file has fatal semantic errors, may be corrupt or incompatible with the "
                   "current version of the program. Check the logs for more details.", Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
        }
        catch (...)
        {
            msgBox("Error", "Failed to load project file: the file has fatal semantic errors, may be corrupt or incompatible with the "
                   "current version of the program.", Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
        }

        Internal::SerializationManager::get().finish();
    }
    else
    {
        msgBox("Error", "Could not load project file: the file has JSON syntax errors or has an invalid format.",
               Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Error);
    }

    is.close();
}


void Application::onLoad(Falcor::RenderContext* pRenderContext)
{
    for (const auto& pModule : getAllModules())
    {
        pModule->init(pRenderContext);
    }
}


void Application::onShutdown()
{
    for (const auto& pModule : getAllModules())
    {
        pModule->cleanup();
    }
}


void Application::onResize(uint32_t width, uint32_t height)
{
    SampleApp::onResize(width, height);
    mUI.setWindowSize({ width, height });
}


void Application::onFrameRender(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo)
{
    EventManager::get().handleEnqueuedEvents();
    EventManager::get().dispatchEvent<Core::EventFrameWillBegin>();

    for (const auto& pModule : getAllModules())
    {
        pModule->update(pRenderContext, pTargetFbo);
    }

    EventManager::get().dispatchEvent<Core::EventFrameEnded>();
}


void Application::onGuiRender(Falcor::Gui* pGui)
{
    // Render the UI for the Core modules
    mUI.render(pGui);
}


bool Application::onKeyEvent(const Falcor::KeyboardEvent& keyEvent)
{
    EventManager::get().dispatchEvent<Core::KeyboardEvent>(keyEvent);
    return Core::KeyboardEvent::handled;
}


bool Application::onMouseEvent(const Falcor::MouseEvent& mouseEvent)
{
    EventManager::get().dispatchEvent<Core::MouseEvent>(mouseEvent);
    return Core::MouseEvent::handled;
}


bool Application::onGamepadState(const Falcor::GamepadState& gamepadState)
{
    EventManager::get().dispatchEvent<Core::GamepadEvent>(gamepadState);
    return Core::GamepadEvent::handled;
}


void Application::onHotReload(Falcor::HotReloadFlags reloaded) {}


ModuleContainerId Application::getModuleContainerId() const
{
    return "GraphEx";
}
