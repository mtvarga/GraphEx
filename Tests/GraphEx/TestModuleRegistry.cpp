#include "GraphExTests.h"


namespace GraphEx::Test
{

struct TestModuleNoConstructorParams : Module
{
    explicit TestModuleNoConstructorParams(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestModule";
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


TEST(ModuleRegistry, RegisterSingleModuleNoConstructorParams)
{
    auto testContainer = TestModuleContainer();
    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestModuleNoConstructorParams>(testContainer.getModuleContainerId(), &testContainer);
    EXPECT_TRUE(ModuleRegistry::get().containerHasModule(testContainer.getModuleContainerId(), pTestModule->getModuleId()));
    EXPECT_EQ(ModuleRegistry::get().getModuleForContainer(testContainer.getModuleContainerId(), pTestModule->getModuleId()), pTestModule);

    cleanup();
}


TEST(ModuleRegistry, RegisterModuleWithConstructorParams)
{
    auto testContainer = TestModuleContainer();
    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestModuleWithParams>(testContainer.getModuleContainerId(), &testContainer, 42);
    EXPECT_TRUE(ModuleRegistry::get().containerHasModule(testContainer.getModuleContainerId(), pTestModule->getModuleId()));
    EXPECT_EQ(ModuleRegistry::get().getModuleForContainer(testContainer.getModuleContainerId(), pTestModule->getModuleId()), pTestModule);
    EXPECT_EQ(std::static_pointer_cast<TestModuleWithParams>(pTestModule)->mParam, 42);

    cleanup();
}


TEST(ModuleRegistry, RegisterMultipleModules)
{
    auto testContainer = TestModuleContainer();

    // Register a module without constructor parameters
    const auto pTestModuleNoParams = ModuleRegistry::get().registerModuleForContainer<TestModuleNoConstructorParams>(testContainer.getModuleContainerId(), &testContainer);
    EXPECT_TRUE(ModuleRegistry::get().containerHasModule(testContainer.getModuleContainerId(), pTestModuleNoParams->getModuleId()));
    EXPECT_EQ(ModuleRegistry::get().getModuleForContainer(testContainer.getModuleContainerId(), pTestModuleNoParams->getModuleId()), pTestModuleNoParams);

    // Register a module with constructor parameters
    const auto pTestModuleWithParams = ModuleRegistry::get().registerModuleForContainer<TestModuleWithParams>(testContainer.getModuleContainerId(), &testContainer, 42);
    EXPECT_TRUE(ModuleRegistry::get().containerHasModule(testContainer.getModuleContainerId(), pTestModuleWithParams->getModuleId()));
    EXPECT_EQ(ModuleRegistry::get().getModuleForContainer(testContainer.getModuleContainerId(), pTestModuleWithParams->getModuleId()), pTestModuleWithParams);
    EXPECT_EQ(std::static_pointer_cast<TestModuleWithParams>(pTestModuleWithParams)->mParam, 42);

    // Verify both modules are registered in the container
    const auto moduleIds = ModuleRegistry::get().getModuleIdsForContainer(testContainer.getModuleContainerId());
    ASSERT_TRUE(moduleIds.has_value());
    EXPECT_EQ(moduleIds->get().size(), 2);
    EXPECT_TRUE(moduleIds->get().count(pTestModuleNoParams->getModuleId()) > 0);
    EXPECT_TRUE(moduleIds->get().count(pTestModuleWithParams->getModuleId()) > 0);

    cleanup();
}



TEST(ModuleRegistry, GetModuleIdsForContainer)
{
    auto testContainer = TestModuleContainer();
    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestModuleNoConstructorParams>(testContainer.getModuleContainerId(), &testContainer);
    const auto moduleIds = ModuleRegistry::get().getModuleIdsForContainer(testContainer.getModuleContainerId());
    ASSERT_TRUE(moduleIds.has_value());
    EXPECT_EQ(moduleIds->get().size(), 1);
    EXPECT_TRUE(moduleIds->get().count(pTestModule->getModuleId()) > 0);

    cleanup();
}


TEST(ModuleRegistry, GetModuleIdForTypeId)
{
    auto testContainer = TestModuleContainer();
    ModuleRegistry::get().registerModuleForContainer<TestModuleNoConstructorParams>(testContainer.getModuleContainerId(), &testContainer);
    const auto& typeId = typeid(TestModuleNoConstructorParams);
    const auto moduleId = ModuleRegistry::get().getModuleIdForTypeId(typeId);
    ASSERT_TRUE(moduleId.has_value());
    EXPECT_EQ(moduleId.value(), "GraphEx.Test.TestModule");

    cleanup();
}


TEST(ModuleRegistry, GetUnregisteredModule)
{
    const auto testContainer = TestModuleContainer();
    const auto moduleId = "NonExistentModule";
    EXPECT_EQ(ModuleRegistry::get().getModuleForContainer(testContainer.getModuleContainerId(), moduleId), nullptr);
    EXPECT_FALSE(ModuleRegistry::get().containerHasModule(testContainer.getModuleContainerId(), moduleId));

    cleanup();
}


TEST(ModuleRegistry, RegisterDuplicateModule)
{
    auto testContainer = TestModuleContainer();
    ModuleRegistry::get().registerModuleForContainer<TestModuleNoConstructorParams>(testContainer.getModuleContainerId(), &testContainer);
    EXPECT_THROW(
        ModuleRegistry::get().registerModuleForContainer<TestModuleNoConstructorParams>(testContainer.getModuleContainerId(), &testContainer),
        Falcor::Exception
    );

    cleanup();
}


TEST(ModuleRegistry, EmptyContainer)
{
    const auto testContainer = TestModuleContainer();
    const auto moduleIds = ModuleRegistry::get().getModuleIdsForContainer(testContainer.getModuleContainerId());
    EXPECT_FALSE(moduleIds.has_value());

    cleanup();
}


TEST(ModuleRegistry, InvalidContainerId)
{
    const auto invalidContainerId = "InvalidContainer";
    const auto moduleIds = ModuleRegistry::get().getModuleIdsForContainer(invalidContainerId);
    EXPECT_FALSE(moduleIds.has_value());

    cleanup();
}


TEST(ModuleRegistry, InvalidTypeId)
{
    const auto& invalidTypeId = typeid(int);
    const auto moduleId = ModuleRegistry::get().getModuleIdForTypeId(invalidTypeId);
    EXPECT_FALSE(moduleId.has_value());

    cleanup();
}


TEST(ModuleRegistry, Cleanup)
{
    auto testContainer = TestModuleContainer();
    ModuleRegistry::get().registerModuleForContainer<TestModuleNoConstructorParams>(testContainer.getModuleContainerId(), &testContainer);
    ModuleRegistry::get().cleanup();

    const auto moduleIds = ModuleRegistry::get().getModuleIdsForContainer(testContainer.getModuleContainerId());
    EXPECT_FALSE(moduleIds.has_value());

    cleanup();
}

} // namespace GraphEx::Test
