#pragma once

#include <GraphEx/GraphEx.h>
#include <gtest/gtest.h>


namespace GraphEx::Test
{

struct TestModuleContainer : ModuleContainer<Module>
{
    ModuleContainerId getModuleContainerId() const override;
    std::vector<std::shared_ptr<Module>> getAllModules() const override;
};


void cleanup();


struct TestApplication : Application
{
    explicit TestApplication(const Falcor::SampleAppConfig& config)
        : Application(config) {}

    void onFrameRender(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override;
};

} // namespace GraphEx::Test
