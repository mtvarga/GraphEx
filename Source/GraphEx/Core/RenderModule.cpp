#include "RenderModule.h"

#include "RenderManager.h"


using namespace GraphEx;
using namespace GraphEx::Core;


RenderModuleBase::RenderModuleBase(ModuleContainerBase* pContainer)
    : Module(pContainer), Requires(pContainer) {}


bool RenderModuleBase::hasGlobalSettings() const
{
    return false;
}


void RenderModuleBase::renderGlobalSettingsGui(Falcor::Gui::Widgets&) {}
void RenderModuleBase::update(Falcor::RenderContext*, const Falcor::ref<Falcor::Fbo>&) {}
