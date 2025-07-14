#include "GraphExTests.h"


namespace GraphEx::Test
{

struct TestSerializableModuleState : ModuleState
{
    int intValue = 0;
    std::string stringValue;

    template<typename Archive>
    void serialize(Archive& ar)
    {
        ar(SerializeNamed<Archive>("intValue", intValue));
        ar(SerializeNamed<Archive>("stringValue", stringValue));
    }
};


struct TestAnotherSerializableModuleState : ModuleState
{
    float floatValue = 0.0f;

    template<typename Archive>
    void serialize(Archive& ar)
    {
        ar(SerializeNamed<Archive>("floatValue", floatValue));
    }
};


struct UnregisteredState : ModuleState
{
    bool boolValue = false;

    template<typename Archive>
    void serialize(Archive& ar)
    {
        ar(SerializeNamed<Archive>("boolValue", boolValue));
    }
};


struct ErroneousState : ModuleState
{
    int intValue = 0;

    template<typename Archive>
    void serialize(Archive&)
    {
        throw Falcor::Exception("Meow.");
    }
};


struct TestSerializableModule : Module, HasSerializableState<TestSerializableModuleState>
{
    explicit TestSerializableModule(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestSerializableModule";
    }
};


struct TestAnotherSerializableModule : Module, HasSerializableState<TestAnotherSerializableModuleState>
{
    explicit TestAnotherSerializableModule(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestAnotherSerializableModule";
    }
};


struct TestAnotherSerializableModuleWithSameStateType : Module, HasSerializableState<TestSerializableModuleState>
{
    explicit TestAnotherSerializableModuleWithSameStateType(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestAnotherSerializableModuleWithSameStateType";
    }
};


struct TestUnregisteredSerializableModule : Module, HasSerializableState<TestAnotherSerializableModuleState>
{
    explicit TestUnregisteredSerializableModule(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestUnregisteredSerializableModule";
    }
};


struct TestSerializableModuleWithUnregisteredState : Module, HasSerializableState<UnregisteredState>
{
    explicit TestSerializableModuleWithUnregisteredState(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestSerializableModuleWithUnregisteredState";
    }
};


struct TestSerializableModuleWithErroneousSerialization : Module, HasSerializableState<ErroneousState>
{
    explicit TestSerializableModuleWithErroneousSerialization(ModuleContainerBase* pContainer)
        : Module(pContainer) {}

    void init(Falcor::RenderContext* pRenderContext) override {}
    void update(Falcor::RenderContext* pRenderContext, const Falcor::ref<Falcor::Fbo>& pTargetFbo) override {}
    void cleanup() override {}

    ModuleId getModuleId() const override
    {
        return "GraphEx.Test.TestSerializableModuleWithErroneousSerialization";
    }
};


TEST(ModuleSerialization, SaveAndLoadModuleStates)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    auto pState = std::make_shared<TestSerializableModuleState>();
    pState->intValue = 42;
    pState->stringValue = "Test String";
    pTestModule->setState(pState);

    std::ostringstream oss;
    {
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().saveModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    cleanup();

    ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(testContainer.getModuleContainerId(), &testContainer);

    {
        std::istringstream iss(oss.str());
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().loadModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    const auto pRestoredModule = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule->getModuleId()
    );

    ASSERT_NE(pRestoredModule, nullptr);

    const auto pRestoredState = std::static_pointer_cast<TestSerializableModule>(pRestoredModule)->getState();
    EXPECT_EQ(pRestoredState->intValue, 42);
    EXPECT_EQ(pRestoredState->stringValue, "Test String");

    cleanup();
}

TEST(ModuleSerialization, SaveAndLoadEmptyState)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    std::ostringstream oss;
    {
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().saveModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    cleanup();

    ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    {
        std::istringstream iss(oss.str());
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().loadModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    const auto pRestoredModule = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule->getModuleId()
    );

    ASSERT_NE(pRestoredModule, nullptr);

    const auto pRestoredState = std::static_pointer_cast<TestSerializableModule>(pRestoredModule)->getState();
    EXPECT_EQ(pRestoredState->intValue, 0);
    EXPECT_EQ(pRestoredState->stringValue, "");

    cleanup();
}

TEST(ModuleSerialization, SaveAndLoadMultipleModules)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule1 = ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    const auto pTestModule2 = ModuleRegistry::get().registerModuleForContainer<TestAnotherSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    // Set states for the modules
    auto pState1 = std::make_shared<TestSerializableModuleState>();
    pState1->intValue = 1;
    pState1->stringValue = "Module1";
    pTestModule1->setState(pState1);

    auto pState2 = std::make_shared<TestAnotherSerializableModuleState>();
    pState2->floatValue = 2.0f;
    pTestModule2->setState(pState2);

    std::ostringstream oss;
    {
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().saveModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    cleanup();

    ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(testContainer.getModuleContainerId(), &testContainer);
    ModuleRegistry::get().registerModuleForContainer<TestAnotherSerializableModule>(testContainer.getModuleContainerId(), &testContainer);

    {
        std::istringstream iss(oss.str());
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().loadModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    const auto pRestoredModule1 = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule1->getModuleId()
    );

    const auto pRestoredModule2 = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule2->getModuleId()
    );

    ASSERT_NE(pRestoredModule1, nullptr);
    ASSERT_NE(pRestoredModule2, nullptr);

    const auto pRestoredState1 = std::static_pointer_cast<TestSerializableModule>(pRestoredModule1)->getState();
    const auto pRestoredState2 = std::static_pointer_cast<TestAnotherSerializableModule>(pRestoredModule2)->getState();

    EXPECT_EQ(pRestoredState1->intValue, 1);
    EXPECT_EQ(pRestoredState1->stringValue, "Module1");
    EXPECT_EQ(pRestoredState2->floatValue, 2.0f);

    cleanup();
}


TEST(ModuleSerialization, SaveAndLoadUnregisteredState)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestSerializableModuleWithUnregisteredState>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    auto pState = std::make_shared<UnregisteredState>();
    pState->boolValue = true;
    pTestModule->setState(pState);

    std::ostringstream oss;
    {
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().saveModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    cleanup();

    ModuleRegistry::get().registerModuleForContainer<TestSerializableModuleWithUnregisteredState>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    {
        std::istringstream iss(oss.str());
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().loadModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    const auto pRestoredModule = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule->getModuleId()
    );

    ASSERT_NE(pRestoredModule, nullptr);

    const auto pRestoredState = std::static_pointer_cast<TestSerializableModuleWithUnregisteredState>(pRestoredModule)->getState();
    EXPECT_EQ(pRestoredState->boolValue, false);

    cleanup();
}

TEST(ModuleSerialization, SaveAndLoadStateAndUnregisteredState)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule1 = ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    const auto pTestModule2 = ModuleRegistry::get().registerModuleForContainer<TestSerializableModuleWithUnregisteredState>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    // Set states for the modules
    auto pState1 = std::make_shared<TestSerializableModuleState>();
    pState1->intValue = 1;
    pState1->stringValue = "Module1";
    pTestModule1->setState(pState1);

    auto pState2 = std::make_shared<UnregisteredState>();
    pState2->boolValue = true;
    pTestModule2->setState(pState2);

    std::ostringstream oss;
    {
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().saveModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    // Clear the module registry
    cleanup();

    // Re-register the same modules
    ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(testContainer.getModuleContainerId(), &testContainer);
    ModuleRegistry::get().registerModuleForContainer<TestSerializableModuleWithUnregisteredState>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    {
        std::istringstream iss(oss.str());
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().loadModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    // Verify the states of both modules were restored
    const auto pRestoredModule1 = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule1->getModuleId()
    );

    const auto pRestoredModule2 = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule2->getModuleId()
    );

    ASSERT_NE(pRestoredModule1, nullptr);
    ASSERT_NE(pRestoredModule2, nullptr);

    const auto pRestoredState1 = std::static_pointer_cast<TestSerializableModule>(pRestoredModule1)->getState();
    const auto pRestoredState2 = std::static_pointer_cast<TestSerializableModuleWithUnregisteredState>(pRestoredModule2)->getState();

    EXPECT_EQ(pRestoredState1->intValue, 1);
    EXPECT_EQ(pRestoredState1->stringValue, "Module1");
    EXPECT_EQ(pRestoredState2->boolValue, false);

    cleanup();
}


TEST(ModuleSerialization, SaveWithError)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestSerializableModuleWithErroneousSerialization>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    auto exceptionThrown = false;

    {
        std::ostringstream oss;
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);

        try
        {
            ModuleRegistry::get().saveModuleStates(*archive);
        }
        catch (const Falcor::Exception& e)
        {
            exceptionThrown = true;
            EXPECT_EQ(std::string(e.what()), "Meow.");
        }

        Internal::SerializationManager::get().finish();
    }

    cleanup();

    EXPECT_TRUE(exceptionThrown);
}


TEST(ModuleSerialization, LoadWithError)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule = ModuleRegistry::get().registerModuleForContainer<TestSerializableModuleWithErroneousSerialization>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    const auto pState = std::make_shared<ErroneousState>();
    pState->intValue = 42;
    pTestModule->setState(pState);

    auto exceptionThrown = false;

    {
        const auto projectFileText = "{ "
            "\"moduleStates\": [ "
                "{ "
                    "\"polymorphic_safe_anchor_tag\": \"GraphEx.Test.TestSerializableModuleWithErroneousSerialization\", "
                    "\"polymorphic_safe_anchor\": { "
                        "\"polymorphic_id\": 2147483649, "
                        "\"polymorphic_name\": \"GraphEx::Test::ErroneousState\", "
                        "\"ptr_wrapper\": { "
                            "\"id\": 2147483649, "
                            "\"data\": { "
                                "\"intValue\": 5 "
                            "} "
                        "} "
                    "} "
                "} "
            "] "
        "}";

        std::istringstream iss(projectFileText);
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);

        try
        {
            ModuleRegistry::get().saveModuleStates(*archive);
        }
        catch (const Falcor::Exception& e)
        {
            exceptionThrown = true;
            EXPECT_EQ(std::string(e.what()), "Meow.");
        }

        Internal::SerializationManager::get().finish();
    }

    const auto pRestoredModule = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule->getModuleId()
    );

    ASSERT_NE(pRestoredModule, nullptr);

    const auto pRestoredState = std::static_pointer_cast<TestSerializableModuleWithErroneousSerialization>(pRestoredModule)->getState();
    EXPECT_EQ(pRestoredState->intValue, 42);
    EXPECT_TRUE(exceptionThrown);

    cleanup();
}


TEST(ModuleSerialization, LoadMoreThanSaved)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule1 = ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    // Set states for the modules
    auto pState1 = std::make_shared<TestSerializableModuleState>();
    pState1->intValue = 1;
    pState1->stringValue = "Module1";
    pTestModule1->setState(pState1);

    std::ostringstream oss;
    {
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().saveModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    cleanup();

    ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(testContainer.getModuleContainerId(), &testContainer);

    const auto pTestModule2 = ModuleRegistry::get().registerModuleForContainer<TestAnotherSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    auto pState2 = std::make_shared<TestAnotherSerializableModuleState>();
    pState2->floatValue = 69.420f;
    pTestModule2->setState(pState2);

    {
        std::istringstream iss(oss.str());
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().loadModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    const auto pRestoredModule1 = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule1->getModuleId()
    );

    const auto pRestoredModule2 = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule2->getModuleId()
    );

    ASSERT_NE(pRestoredModule1, nullptr);
    ASSERT_NE(pRestoredModule2, nullptr);

    const auto pRestoredState1 = std::static_pointer_cast<TestSerializableModule>(pRestoredModule1)->getState();
    const auto pRestoredState2 = std::static_pointer_cast<TestAnotherSerializableModule>(pRestoredModule2)->getState();

    EXPECT_EQ(pRestoredState1->intValue, 1);
    EXPECT_EQ(pRestoredState1->stringValue, "Module1");
    EXPECT_EQ(pRestoredState2->floatValue, 69.420f);

    cleanup();
}


TEST(ModuleSerialization, LoadLessThanSaved)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule1 = ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    const auto pTestModule2 = ModuleRegistry::get().registerModuleForContainer<TestAnotherSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    // Set states for the modules
    auto pState1 = std::make_shared<TestSerializableModuleState>();
    pState1->intValue = 1;
    pState1->stringValue = "Module1";
    pTestModule1->setState(pState1);

    auto pState2 = std::make_shared<TestAnotherSerializableModuleState>();
    pState2->floatValue = 69.420f;
    pTestModule2->setState(pState2);

    std::ostringstream oss;
    {
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().saveModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    cleanup();

    ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(testContainer.getModuleContainerId(), &testContainer);

    const auto pTestModule3 = ModuleRegistry::get().registerModuleForContainer<TestUnregisteredSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    auto pState3 = std::make_shared<TestAnotherSerializableModuleState>();
    pState3->floatValue = 420.69f;
    pTestModule3->setState(pState3);

    {
        std::istringstream iss(oss.str());
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().loadModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    const auto pRestoredModule1 = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule1->getModuleId()
    );

    const auto pRestoredModule2 = ModuleRegistry::get().getModuleForContainer(
        testContainer.getModuleContainerId(),
        pTestModule3->getModuleId()
    );

    ASSERT_NE(pRestoredModule1, nullptr);

    const auto pRestoredState1 = std::static_pointer_cast<TestSerializableModule>(pRestoredModule1)->getState();
    const auto pRestoredState2 = std::static_pointer_cast<TestUnregisteredSerializableModule>(pRestoredModule2)->getState();

    EXPECT_EQ(pRestoredState1->intValue, 1);
    EXPECT_EQ(pRestoredState1->stringValue, "Module1");
    EXPECT_EQ(pRestoredState2->floatValue, 420.69f);

    cleanup();
}


TEST(ModuleSerialization, SaveAndLoadMultipleWithSameStateType)
{
    auto testContainer = TestModuleContainer();

    const auto pTestModule1 = ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    const auto pTestModule2 = ModuleRegistry::get().registerModuleForContainer<TestAnotherSerializableModuleWithSameStateType>(
        testContainer.getModuleContainerId(),
        &testContainer
    );

    // Set states for the modules
    auto pState1 = std::make_shared<TestSerializableModuleState>();
    pState1->intValue = 1;
    pState1->stringValue = "Module1";
    pTestModule1->setState(pState1);

    auto pState2 = std::make_shared<TestSerializableModuleState>();
    pState2->intValue = 2;
    pState2->stringValue = "Module2";
    pTestModule2->setState(pState2);

    std::ostringstream oss;
    {
        auto archive = Internal::SerializationManager::get().beginSave(oss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().saveModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    // Clear the module registry
    cleanup();

    // Re-register the same modules
    ModuleRegistry::get().registerModuleForContainer<TestSerializableModule>(testContainer.getModuleContainerId(), &testContainer);
    ModuleRegistry::get().registerModuleForContainer<TestAnotherSerializableModuleWithSameStateType>(testContainer.getModuleContainerId(), &testContainer);

    {
        std::istringstream iss(oss.str());
        auto archive = Internal::SerializationManager::get().beginLoad(iss);
        ASSERT_NE(archive, std::nullopt);
        ModuleRegistry::get().loadModuleStates(*archive);
        Internal::SerializationManager::get().finish();
    }

    // Verify the states of both modules were restored
    const auto pRestoredModule1 = ModuleRegistry::get().getModuleForContainer(testContainer.getModuleContainerId(), pTestModule1->getModuleId());
    const auto pRestoredModule2 = ModuleRegistry::get().getModuleForContainer(testContainer.getModuleContainerId(), pTestModule2->getModuleId());

    ASSERT_NE(pRestoredModule1, nullptr);
    ASSERT_NE(pRestoredModule2, nullptr);

    const auto pRestoredState1 = std::static_pointer_cast<TestSerializableModule>(pRestoredModule1)->getState();
    const auto pRestoredState2 = std::static_pointer_cast<TestAnotherSerializableModuleWithSameStateType>(pRestoredModule2)->getState();

    EXPECT_EQ(pRestoredState1->intValue, 1);
    EXPECT_EQ(pRestoredState1->stringValue, "Module1");
    EXPECT_EQ(pRestoredState2->intValue, 2);
    EXPECT_EQ(pRestoredState2->stringValue, "Module2");

    cleanup();
}

} // namespace GraphEx::Test


GRAPHEX_REGISTER_MODULE_STATE(GraphEx::Test::TestSerializableModuleState);
GRAPHEX_REGISTER_MODULE_STATE(GraphEx::Test::TestAnotherSerializableModuleState);
GRAPHEX_REGISTER_MODULE_STATE(GraphEx::Test::ErroneousState);
