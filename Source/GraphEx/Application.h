/***************************************************************************
 # Copyright (c) 2015-21, NVIDIA CORPORATION. All rights reserved.
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
#pragma once

// Falcor includes
#include <Core/SampleApp.h>

// GraphEx includes
#include "API/Module.h"
#include "UI/UI.h"


namespace GraphEx
{

struct GRAPHEX_EXPORTABLE Application : Falcor::SampleApp, ModuleContainer<Module>
{
    explicit Application(const Falcor::SampleAppConfig& config);

    void onLoad(Falcor::RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override;
    void onGuiRender(Falcor::Gui* pGui) override;
    bool onKeyEvent(const Falcor::KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const Falcor::MouseEvent& mouseEvent) override;
    bool onGamepadState(const Falcor::GamepadState& gamepadState) override;
    void onHotReload(Falcor::HotReloadFlags reloaded) override;

    ModuleContainerId getModuleContainerId() const override;

    void registerCoreEvents();
    void registerCoreModules();

    void saveProject();
    void saveProjectAs(const std::filesystem::path& filePath);
    void loadProject(const std::filesystem::path& filePath);

private:
    std::filesystem::path mProjectFilePath{ "" };
    UI mUI;

public:
    DEFAULT_CONST_GETREF_DEFINITION(ProjectFilePath, mProjectFilePath)
    DEFAULT_CONST_GETREF_DEFINITION(UI, mUI)
};

} // namespace GraphEx
