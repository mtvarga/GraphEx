#include "GraphExTests.h"


using namespace GraphEx;
using namespace GraphEx::Test;


ModuleContainerId TestModuleContainer::getModuleContainerId() const
{
    return "GraphEx.Test.TestModuleContainer";
}


std::vector<std::shared_ptr<Module>> TestModuleContainer::getAllModules() const
{
    return ModuleContainer::getAllModules();
}


void Test::cleanup()
{
    ModuleRegistry::get().cleanup();
    EventManager::get().cleanup();
}


void TestApplication::onFrameRender(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo)
{
    Application::onFrameRender(pRenderContext, pTargetFbo);
    shutdown();
}
