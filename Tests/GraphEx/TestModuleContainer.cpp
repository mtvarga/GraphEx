#include "GraphExTests.h"


namespace GraphEx::Test
{

struct TestModuleNoParams : Module
{
    explicit TestModuleNoParams(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestModuleNoParams";
    }
};

struct TestModuleWithParams : Module
{
    explicit TestModuleWithParams(ModuleContainerBase* pContainer, const int param)
        : Module(pContainer), mParam(param) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestModuleWithParams";
    }

    int mParam;
};

TEST(ModuleContainer, RegisterModule)
{
    auto testContainer = TestModuleContainer();
    testContainer.registerModule<TestModuleNoParams>();
    EXPECT_TRUE(testContainer.contains<TestModuleNoParams>());
    cleanup();
}

TEST(ModuleContainer, RegisterContainerModule)
{
    auto testContainer = TestModuleContainer();
    testContainer.registerModule<TestModuleWithParams>(42);
    EXPECT_TRUE(testContainer.contains<TestModuleWithParams>());
    EXPECT_EQ(testContainer.getContained<TestModuleWithParams>().mParam, 42);
    cleanup();
}

TEST(ModuleContainer, HasModule)
{
    auto testContainer = TestModuleContainer();
    testContainer.registerModule<TestModuleNoParams>();
    EXPECT_TRUE(testContainer.contains<TestModuleNoParams>());
    EXPECT_FALSE(testContainer.contains<TestModuleWithParams>());
    cleanup();
}

TEST(ModuleContainer, GetModule)
{
    auto testContainer = TestModuleContainer();
    testContainer.registerModule<TestModuleNoParams>();
    const auto& module = testContainer.getContained<TestModuleNoParams>();
    EXPECT_EQ(module.getModuleId(), "GraphEx.Test.TestModuleNoParams");
    cleanup();
}

TEST(ModuleContainer, GetAllModules)
{
    auto testContainer = TestModuleContainer();
    testContainer.registerModule<TestModuleNoParams>();
    testContainer.registerModule<TestModuleWithParams>(42);

    auto allModules = testContainer.getAllModules();
    EXPECT_EQ(allModules.size(), 2);
    EXPECT_TRUE(std::any_of(allModules.begin(), allModules.end(), [](const auto& module) {
        return module->getModuleId() == "GraphEx.Test.TestModuleNoParams";
    }));
    EXPECT_TRUE(std::any_of(allModules.begin(), allModules.end(), [](const auto& module) {
        return module->getModuleId() == "GraphEx.Test.TestModuleWithParams";
    }));
    cleanup();
}

TEST(ModuleContainer, OnModuleRegistered)
{
    struct TestContainer : ModuleContainer<Module>
    {
        ModuleContainerId getModuleContainerId() const override
        {
            return "GraphEx.Test.TestContainer";
        }

        void onModuleRegistered(const std::shared_ptr<Module>& pModule) override
        {
            ModuleContainer::onModuleRegistered(pModule);
            EXPECT_EQ(pModule->getModuleId(), "GraphEx.Test.TestModuleNoParams");
        }
    };

    TestContainer testContainer;
    testContainer.registerModule<TestModuleNoParams>();
    cleanup();
}

TEST(ModuleContainer, GetMaybeContained)
{
    auto testContainer = TestModuleContainer();
    testContainer.registerModule<TestModuleNoParams>();

    const auto pMaybeModule = testContainer.getMaybeContained<TestModuleNoParams>();
    EXPECT_NE(pMaybeModule, nullptr);
    EXPECT_EQ(pMaybeModule->getModuleId(), "GraphEx.Test.TestModuleNoParams");

    const auto pNonExistentModule = testContainer.getMaybeContained<TestModuleWithParams>();
    EXPECT_EQ(pNonExistentModule, nullptr);
    cleanup();
}

TEST(ModuleContainer, ContainsAny)
{
    auto testContainer = TestModuleContainer();
    testContainer.registerModule<TestModuleNoParams>();

    EXPECT_TRUE((testContainer.containsAny<TestModuleNoParams, TestModuleWithParams>()));
    EXPECT_FALSE(testContainer.containsAny<TestModuleWithParams>());
    cleanup();
}

TEST(ModuleContainer, ContainsAll)
{
    struct AnotherModule : Module
    {
        explicit AnotherModule(ModuleContainerBase* pContainer)
            : Module(pContainer) {}

        void init(Falcor::RenderContext* pRenderContext) override {}
        void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
        void cleanup() override {}

        ModuleId getModuleId() const override
        {
            return "GraphEx.Test.AnotherModule";
        }
    };

    auto testContainer = TestModuleContainer();
    testContainer.registerModule<TestModuleNoParams>();
    testContainer.registerModule<TestModuleWithParams>(42);

    EXPECT_TRUE((testContainer.containsAll<TestModuleNoParams, TestModuleWithParams>()));
    EXPECT_FALSE((testContainer.containsAll<TestModuleNoParams, TestModuleWithParams, AnotherModule>()));
    cleanup();
}

TEST(ModuleContainer, InvalidModuleAccess)
{
    const auto testContainer = TestModuleContainer();
    EXPECT_THROW(testContainer.getContained<TestModuleNoParams>(), Falcor::Exception);
    cleanup();
}

} // namespace GraphEx::Test
