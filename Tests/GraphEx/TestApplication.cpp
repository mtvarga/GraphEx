#include "GraphExTests.h"
#include "Core/Plugin.h"

using namespace GraphEx;


static std::unique_ptr<Test::TestApplication> initApplication()
{
    Falcor::SampleAppConfig config;
    config.headless = true;
    config.showUI = false;
    return std::make_unique<Test::TestApplication>(config);
}


namespace GraphEx::Test
{

struct CustomEvent : Event<void()> {};

struct CustomModule : Module
{
    explicit CustomModule(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override
    {
        mInitCalled = true;
    }

    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override
    {
        mUpdateCalled = true;
    }

    void cleanup() override
    {
        mCleanupCalled = true;
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.CustomModule";
    }

private:
    bool mInitCalled = false, mUpdateCalled = false, mCleanupCalled = false;

public:
    DEFAULT_CONST_GETTER_DEFINITION(InitCalled, mInitCalled)
    DEFAULT_CONST_GETTER_DEFINITION(UpdateCalled, mUpdateCalled)
    DEFAULT_CONST_GETTER_DEFINITION(CleanupCalled, mCleanupCalled)
};


/*TEST(Application, DISABLED_BlankRun)
{
    auto pApp = initApplication();
    EXPECT_NO_THROW(pApp->run());
    pApp.reset();
    cleanup();
}


TEST(Application, DISABLED_ModuleCallbacks)
{
    auto pApp = initApplication();
    pApp->registerModule<CustomModule>();
    pApp->run();

    const auto& module = pApp->getContained<CustomModule>();
    EXPECT_TRUE(module.getInitCalled());
    EXPECT_TRUE(module.getUpdateCalled());
    EXPECT_TRUE(module.getCleanupCalled());

    cleanup();
}


TEST(Application, DISABLED_CoreEvents)
{
    constexpr auto CORE_EVENT_COUNT = 6;  // Note: keyboard, mouse and gamepad events cannot be reproduced in testing env

    auto pApp = initApplication();
    auto handlerCallCounter = 0;
    const std::function handler = [&handlerCallCounter] { ++handlerCallCounter; };

    EventManager::get().registerEventHandler<Core::EventFrameWillBegin>(handler);
    EventManager::get().registerEventHandler<Core::EventRenderWillBegin>(handler);
    EventManager::get().registerEventHandler<Core::EventRenderBegan>(handler);
    EventManager::get().registerEventHandler<Core::EventRenderWillEnd>(handler);
    EventManager::get().registerEventHandler<Core::EventRenderEnded>(handler);
    EventManager::get().registerEventHandler<Core::EventFrameEnded>(handler);

    EXPECT_NO_THROW(pApp->run());
    EXPECT_EQ(handlerCallCounter, CORE_EVENT_COUNT);
    cleanup();
}


TEST(Application, DISABLED_HandleEnqueuedEvents)
{
    auto pApp = initApplication();
    EventManager::get().registerEvent<CustomEvent>();

    auto eventHandlerCalled = false;
    EventManager::get().registerEventHandler<CustomEvent>([&eventHandlerCalled] { eventHandlerCalled = true; });
    EventManager::get().enqueueEvent<CustomEvent>();

    pApp->run();
    EXPECT_EQ(eventHandlerCalled, true);
    cleanup();
}


TEST(Application, DISABLED_BlankSaveAndLoad)
{
    const auto path = Falcor::getRuntimeDirectory() / "TestProject.gxproj.tmp";
    auto pApp = initApplication();

    {
        const auto& [ cameraManager, sceneManager, renderManager ] =
            pApp->getsContained<Core::CameraManager, Core::SceneManager, Core::RenderManager>();

        cameraManager.getState()->makeNewCameraActive = true;
        sceneManager.getState()->pGlobalLight->setPosition({ 69.0f, 420.0f, 1000.0f });
        renderManager.getState()->backgroundColor = { 0.0f, 1.0f, 1.0f, 1.0f };
    }

    EXPECT_NO_THROW(pApp->saveProjectAs(path));
    EXPECT_TRUE(std::filesystem::exists(path));

    {
        const auto& [ cameraManager, sceneManager, renderManager ] =
            pApp->getsContained<Core::CameraManager, Core::SceneManager, Core::RenderManager>();

        cameraManager.getState()->makeNewCameraActive = true;
        sceneManager.getState()->pGlobalLight->setPosition({ 0.0f, 0.0f, 0.0f });
        renderManager.getState()->backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    EXPECT_NO_THROW(pApp->loadProject(path));

    {
        const auto& [ cameraManager, sceneManager, renderManager ] =
            pApp->getsContained<Core::CameraManager, Core::SceneManager, Core::RenderManager>();

        EXPECT_TRUE(cameraManager.getState()->makeNewCameraActive);
        EXPECT_TRUE((Falcor::math::all(sceneManager.getState()->pGlobalLight->getPosition() == Falcor::float3{69.0f, 420.0f, 1000.0f})));
        EXPECT_TRUE((Falcor::math::all(renderManager.getState()->backgroundColor == Falcor::float4{0.0f, 1.0f, 1.0f, 1.0f})));
    }

    cleanup();

    // Delete the file we created
    if (!std::filesystem::remove(path))
    {
        // "Notify" user that the file was not removed
        FAIL();
    }
}*/

} // namespace GraphEx::Test
