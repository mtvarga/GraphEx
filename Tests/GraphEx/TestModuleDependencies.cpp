#include "GraphExTests.h"


namespace GraphEx::Test
{

struct AnotherTestModuleContainer : ModuleContainer<Module>
{
    explicit AnotherTestModuleContainer(const int value)
        : mValue(value) {}

    int getTestValue() const
    {
        return mValue;
    }

    ModuleContainerId getModuleContainerId() const override
    {
        return "GraphEx.Test.AnotherTestModuleContainer";
    }

private:
    int mValue;
};


struct TestRequiresContainerModule : Module, Requires<Container<AnotherTestModuleContainer>>
{
    explicit TestRequiresContainerModule(ModuleContainerBase* pContainer)
        : Module(pContainer), Requires(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    void testGetter(const int value) const
    {
        EXPECT_EQ(mpContainer->getTestValue(), value);
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestRequiresContainerModule";
    }
};

struct Module1 : Module
{
    explicit Module1(ModuleContainerBase* pContainer, const int value)
        : Module(pContainer), mValue(value) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    int getTestValue() const
    {
        return mValue;
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.Module1";
    }

private:
    int mValue;
};


struct Module2 : Module
{
    explicit Module2(ModuleContainerBase* pContainer, const int value)
        : Module(pContainer), mValue(value) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    int getTestValue() const
    {
        return mValue;
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.Module2";
    }

private:
    int mValue;
};


struct Module3 : Module, Requires<Siblings<Module2>>
{
    explicit Module3(ModuleContainerBase* pContainer, const int value)
        : Module(pContainer), Requires(pContainer), mValue(value) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    int getTestValue() const
    {
        return mValue;
    }

    void testGetter(const int value) const
    {
        EXPECT_EQ(getRequired<Module2>().getTestValue(), value);
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.Module3";
    }

private:
    int mValue;
};


struct TestRequiresOneSiblingModule : Module, Requires<Siblings<Module1>>
{
    explicit TestRequiresOneSiblingModule(ModuleContainerBase* pContainer)
        : Module(pContainer), Requires(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    void testGetter(const int value) const
    {
        EXPECT_EQ(getRequired<Module1>().getTestValue(), value);
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestRequiresOneSiblingModule";
    }
};


struct TestRequiresMultipleSiblingModules : Module, Requires<Siblings<Module1, Module3>>
{
    explicit TestRequiresMultipleSiblingModules(ModuleContainerBase* pContainer)
        : Module(pContainer), Requires(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    void testGetter(const int value1, const int value2, const int value3) const
    {
        EXPECT_EQ(std::tuple_size_v<decltype(getsRequired())>, 2);
        EXPECT_EQ(getRequired<Module1>().getTestValue(), value1);
        EXPECT_EQ(getRequired<Module3>().getTestValue(), value3);

        getRequired<Module3>().testGetter(value2);
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestRequiresMultipleSiblingModules";
    }
};


struct TestRequiresOneAssociatedModule : Module, Requires<Associated<Module1>>
{
    explicit TestRequiresOneAssociatedModule(ModuleContainerBase* pContainer, Module1& module1)
        : Module(pContainer), Requires(pContainer, module1) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    void testGetter(const int value) const
    {
        EXPECT_EQ(getRequired<Module1>().getTestValue(), value);
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestRequiresOneAssociatedModule";
    }
};


struct TestRequiresMultipleAssociatedModules : Module, Requires<Associated<Module1, Module3>>
{
    explicit TestRequiresMultipleAssociatedModules(ModuleContainerBase* pContainer, Module1& module1, Module3& module2)
        : Module(pContainer), Requires(pContainer, module1, module2) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    void testGetter(const int value1, const int value2, const int value3) const
    {
        EXPECT_EQ(std::tuple_size_v<decltype(getsRequired())>, 2);
        EXPECT_EQ(getRequired<Module1>().getTestValue(), value1);
        EXPECT_EQ(getRequired<Module3>().getTestValue(), value3);

        getRequired<Module3>().testGetter(value2);
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestRequiresMultipleAssociatedModules";
    }
};


struct TestRequiresAllTypesOfRequirements : Module, Requires<Associated<Module3>, Siblings<Module1>, Container<AnotherTestModuleContainer>>
{
    explicit TestRequiresAllTypesOfRequirements(ModuleContainerBase* pContainer, Module3& module3)
        : Module(pContainer), Requires(pContainer, module3) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    void testGetter(const int containerValue, const int value1, const int value2, const int value3) const
    {
        EXPECT_EQ(mpContainer->getTestValue(), containerValue);

        EXPECT_EQ(std::tuple_size_v<decltype(getsRequired())>, 2);
        EXPECT_EQ(getRequired<Module1>().getTestValue(), value1);
        EXPECT_EQ(getRequired<Module3>().getTestValue(), value3);

        getRequired<Module3>().testGetter(value2);
    }

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestRequiresAllTypesOfRequirements";
    }
};


TEST(ModuleDependencies, RequiresContainer)
{
    constexpr int TEST_VALUE = 420;

    auto testContainer = TestModuleContainer();
    auto anotherTestContainer = AnotherTestModuleContainer(TEST_VALUE);

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestRequiresContainerModule>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer
    );

    pTestModule->testGetter(TEST_VALUE);

    cleanup();

    EXPECT_THROW(ModuleRegistry::get().registerModuleForContainer<TestRequiresContainerModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    ), Falcor::Exception);

    cleanup();
}


TEST(ModuleDependencies, RequiresOneSibling)
{
    constexpr int TEST_VALUE = 420;
    auto testContainer = TestModuleContainer();

    ModuleRegistry::get().registerModuleForContainer<Module1>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE
    );

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestRequiresOneSiblingModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    pTestModule->testGetter(TEST_VALUE);

    cleanup();

    EXPECT_THROW(ModuleRegistry::get().registerModuleForContainer<TestRequiresOneSiblingModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    ), Falcor::Exception);

    cleanup();
}


TEST(ModuleDependencies, RequiresMultipleSiblings)
{
    constexpr int TEST_VALUE_1 = 420;
    constexpr int TEST_VALUE_2 = 69;
    constexpr int TEST_VALUE_3 = 100;

    auto testContainer = TestModuleContainer();

    ModuleRegistry::get().registerModuleForContainer<Module1>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_1
    );

    ModuleRegistry::get().registerModuleForContainer<Module2>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_2
    );

    ModuleRegistry::get().registerModuleForContainer<Module3>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_3
    );

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestRequiresMultipleSiblingModules>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    pTestModule->testGetter(TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3);

    cleanup();

    // Edge case: Only one requirement is met
    ModuleRegistry::get().registerModuleForContainer<Module1>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_1
    );

    EXPECT_THROW(ModuleRegistry::get().registerModuleForContainer<TestRequiresMultipleSiblingModules>(
        testContainer.getModuleContainerId(),
        &testContainer
    ), Falcor::Exception);

    cleanup();
}


TEST(ModuleDependencies, RequiresOneAssociated)
{
    constexpr int TEST_VALUE = 69;

    auto testContainer = TestModuleContainer();
    auto anotherTestContainer = AnotherTestModuleContainer(69420);

    const auto pAssociated = ModuleRegistry::get().registerModuleForContainer<Module1>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        TEST_VALUE
    );

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestRequiresOneAssociatedModule>(
        testContainer.getModuleContainerId(),
        &testContainer,
        *pAssociated
    );

    pTestModule->testGetter(TEST_VALUE);

    cleanup();

    // If associated modules are required, the only way to construct is to give the references of the associated modules, thus
    // the requirement must be fulfilled at compile time
}


TEST(ModuleDependencies, RequiresMultipleAssociated)
{
    constexpr int TEST_VALUE_1 = 100;
    constexpr int TEST_VALUE_2 = 420;
    constexpr int TEST_VALUE_3 = 69;

    auto testContainer = TestModuleContainer();
    auto anotherTestContainer = AnotherTestModuleContainer(69420);

    const auto pAssociated1 = ModuleRegistry::get().registerModuleForContainer<Module1>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        TEST_VALUE_1
    );

    ModuleRegistry::get().registerModuleForContainer<Module2>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        TEST_VALUE_2
    );

    const auto pAssociated2 = ModuleRegistry::get().registerModuleForContainer<Module3>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        TEST_VALUE_3
    );

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestRequiresMultipleAssociatedModules>(
        testContainer.getModuleContainerId(),
        &testContainer,
        *pAssociated1,
        *pAssociated2
    );

    pTestModule->testGetter(TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3);

    cleanup();

    // If associated modules are required, the only way to construct is to give the references of the associated modules, thus
    // the requirement must be fulfilled at compile time
}


TEST(ModuleDependencies, AllRequirements)
{
    constexpr int CONTAINER_TEST_VALUE = 69420;

    constexpr int TEST_VALUE_1 = 100;
    constexpr int TEST_VALUE_2 = 420;
    constexpr int TEST_VALUE_3 = 69;

    auto testContainer = TestModuleContainer();
    auto anotherTestContainer = AnotherTestModuleContainer(CONTAINER_TEST_VALUE);

    ModuleRegistry::get().registerModuleForContainer<Module2>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_2
    );

    auto pAssociated = ModuleRegistry::get().registerModuleForContainer<Module3>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_3
    );

    ModuleRegistry::get().registerModuleForContainer<Module1>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        TEST_VALUE_1
    );

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestRequiresAllTypesOfRequirements>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        *pAssociated
    );

    pTestModule->testGetter(CONTAINER_TEST_VALUE, TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3);

    cleanup();

    // Case 1: Sibling requirement not met
    ModuleRegistry::get().registerModuleForContainer<Module2>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_2
    );

    pAssociated = ModuleRegistry::get().registerModuleForContainer<Module3>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_3
    );

    EXPECT_THROW(ModuleRegistry::get().registerModuleForContainer<TestRequiresAllTypesOfRequirements>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        *pAssociated
    ), Falcor::Exception);

    cleanup();

    // Case 2: Container requirement not met
    ModuleRegistry::get().registerModuleForContainer<Module2>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        TEST_VALUE_2
    );

    pAssociated = ModuleRegistry::get().registerModuleForContainer<Module3>(
        anotherTestContainer.getModuleContainerId(),
        &anotherTestContainer,
        TEST_VALUE_3
    );

    ModuleRegistry::get().registerModuleForContainer<Module1>(
        testContainer.getModuleContainerId(),
        &testContainer,
        TEST_VALUE_1
    );

    EXPECT_THROW(ModuleRegistry::get().registerModuleForContainer<TestRequiresAllTypesOfRequirements>(
        testContainer.getModuleContainerId(),
        &testContainer,
        *pAssociated
    ), Falcor::Exception);

    cleanup();

    // Case 3: About associated requirement: if associated modules are required, the only way to construct is to give the references of the
    // associated modules, thus the requirement must be fulfilled at compile time
}

} // namespace GraphEx::Test
