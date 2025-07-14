#include "GraphExTests.h"


using namespace GraphEx;


namespace GraphEx::Test
{

struct TestPropertySet : GlocalPropertySet<TestPropertySet>
{
    explicit TestPropertySet(std::shared_ptr<TestPropertySet> pGlobalSet = nullptr)
        : GlocalPropertySet(std::move(pGlobalSet))
        , GlocalInit(testIntProperty, 42)
        , GlocalInit(testBoolProperty, false)
        , GlocalInit(testFloatProperty, 3.14f) {}

    template<typename Archive>
    void serialize(Archive& ar)
    {
        ar(SerializeNamed<Archive>("glocalPropertySet", SerializeBase<GlocalPropertySet>(this)));
        ar(SerializeNamed<Archive>("testIntProperty", testIntProperty));
        ar(SerializeNamed<Archive>("testBoolProperty", testBoolProperty));
        ar(SerializeNamed<Archive>("testFloatProperty", testFloatProperty));
    }

    GlocalProperty<int> testIntProperty;
    GlocalProperty<bool> testBoolProperty;
    GlocalProperty<float> testFloatProperty;
};


TEST(GlobalLocalProperty, DefaultInitialization)
{
    const auto pLocalSet = std::make_shared<TestPropertySet>();

    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 42);
    EXPECT_EQ(GlocalGet(pLocalSet, testBoolProperty), false);
    EXPECT_EQ(GlocalGet(pLocalSet, testFloatProperty), 3.14f);

    EXPECT_FALSE(pLocalSet->shouldUseGlobalFor(pLocalSet->testIntProperty));
    EXPECT_FALSE(pLocalSet->shouldUseGlobalFor(pLocalSet->testBoolProperty));
    EXPECT_FALSE(pLocalSet->shouldUseGlobalFor(pLocalSet->testFloatProperty));
}

TEST(GlobalLocalProperty, UseGlobalProperty)
{
    const auto pGlobalSet = std::make_shared<TestPropertySet>();
    GlocalSet(pGlobalSet, testIntProperty, 100);
    GlocalSet(pGlobalSet, testBoolProperty, true);
    GlocalSet(pGlobalSet, testFloatProperty, 1.23f);

    const auto pLocalSet = std::make_shared<TestPropertySet>(pGlobalSet);
    GlocalSet(pLocalSet, testIntProperty, 42);
    GlocalSet(pLocalSet, testBoolProperty, false);
    GlocalSet(pLocalSet, testFloatProperty, 3.14f);

    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 100);
    EXPECT_EQ(GlocalGet(pLocalSet, testBoolProperty), true);
    EXPECT_EQ(GlocalGet(pLocalSet, testFloatProperty), 1.23f);

    pLocalSet->switchUseGlobalFor(pLocalSet->testIntProperty);
    pLocalSet->switchUseGlobalFor(pLocalSet->testBoolProperty);
    pLocalSet->switchUseGlobalFor(pLocalSet->testFloatProperty);

    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 42);
    EXPECT_EQ(GlocalGet(pLocalSet, testBoolProperty), false);
    EXPECT_EQ(GlocalGet(pLocalSet, testFloatProperty), 3.14f);
}
TEST(GlobalLocalProperty, SwitchBetweenLocalAndGlobal)
{
    const auto pGlobalSet = std::make_shared<TestPropertySet>();
    GlocalSet(pGlobalSet, testIntProperty, 200);

    const auto pLocalSet = std::make_shared<TestPropertySet>(pGlobalSet);
    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 200);
    GlocalSet(pLocalSet, testIntProperty, 42);
    pLocalSet->setUseGlobalFor(pLocalSet->testIntProperty, false);
    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 42);
    GlocalSet(pLocalSet, testIntProperty, 300);
    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 300);

    pLocalSet->setUseGlobalFor(pLocalSet->testIntProperty, true);
    GlocalSet(pLocalSet, testIntProperty, 69);
    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 200);

    pLocalSet->setUseGlobalFor(pLocalSet->testIntProperty, false);
    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 69);
}


TEST(GlobalLocalProperty, NoGlobalSet)
{
    const auto pLocalSet = std::make_shared<TestPropertySet>();

    EXPECT_FALSE(pLocalSet->hasGlobalPropertySet());
    EXPECT_FALSE(pLocalSet->hasPropertiesUsingGlobal());
    EXPECT_FALSE(pLocalSet->shouldUseGlobalFor(pLocalSet->testIntProperty));
    EXPECT_FALSE(pLocalSet->shouldUseGlobalFor(pLocalSet->testBoolProperty));
    EXPECT_FALSE(pLocalSet->shouldUseGlobalFor(pLocalSet->testFloatProperty));

    pLocalSet->setUseGlobalFor(pLocalSet->testIntProperty, true);
    EXPECT_FALSE(pLocalSet->shouldUseGlobalFor(pLocalSet->testIntProperty));
    EXPECT_EQ(GlocalGet(pLocalSet, testIntProperty), 42);
}


TEST(GlobalLocalProperty, GlobalUseCount)
{
    const auto pGlobalSet = std::make_shared<TestPropertySet>();
    GlocalSet(pGlobalSet, testIntProperty, 100);
    GlocalSet(pGlobalSet, testBoolProperty, true);
    GlocalSet(pGlobalSet, testFloatProperty, 1.23f);

    const auto pLocalSet = std::make_shared<TestPropertySet>(pGlobalSet);
    EXPECT_TRUE(pLocalSet->hasPropertiesUsingGlobal());

    pLocalSet->switchUseGlobalFor(pLocalSet->testIntProperty);
    pLocalSet->switchUseGlobalFor(pLocalSet->testBoolProperty);
    pLocalSet->switchUseGlobalFor(pLocalSet->testFloatProperty);

    EXPECT_FALSE(pLocalSet->hasPropertiesUsingGlobal());

    pLocalSet->switchUseGlobalFor(pLocalSet->testIntProperty);

    EXPECT_TRUE(pLocalSet->hasPropertiesUsingGlobal());
}


TEST(GlobalLocalProperty, Serialization)
{
    const auto pGlobalSet = std::make_shared<TestPropertySet>();
    GlocalSet(pGlobalSet, testIntProperty, 100);
    GlocalSet(pGlobalSet, testBoolProperty, true);
    GlocalSet(pGlobalSet, testFloatProperty, 1.23f);

    const auto pLocalSet = std::make_shared<TestPropertySet>(pGlobalSet);
    pLocalSet->setUseGlobalFor(pLocalSet->testBoolProperty, false);
    pLocalSet->setUseGlobalFor(pLocalSet->testFloatProperty, false);

    GlocalSet(pLocalSet, testBoolProperty, false);
    GlocalSet(pLocalSet, testFloatProperty, 4.56f);

    std::ostringstream oss;
    {
        cereal::JSONOutputArchive archive(oss);
        archive(pLocalSet);
    }

    std::shared_ptr<TestPropertySet> pDeserializedSet;
    {
        std::istringstream iss(oss.str());
        cereal::JSONInputArchive archive(iss);
        archive(pDeserializedSet);
    }

    EXPECT_NE(pDeserializedSet, nullptr);
    EXPECT_EQ(GlocalGet(pDeserializedSet, testIntProperty), 100);
    EXPECT_EQ(GlocalGet(pDeserializedSet, testBoolProperty), false);
    EXPECT_EQ(GlocalGet(pDeserializedSet, testFloatProperty), 4.56f);
    EXPECT_TRUE(pDeserializedSet->shouldUseGlobalFor(pDeserializedSet->testIntProperty));
}


TEST(GlobalLocalProperty, ForeignProperty)
{
    const auto pPropertySet1 = std::make_shared<TestPropertySet>();
    const auto pPropertySet2 = std::make_shared<TestPropertySet>();

    EXPECT_THROW(pPropertySet2->setLocalValueFor(pPropertySet1->testIntProperty, 42), Falcor::Exception);
    EXPECT_THROW(pPropertySet2->getLocalValueFor(pPropertySet1->testIntProperty), Falcor::Exception);
    EXPECT_THROW(pPropertySet2->shouldUseGlobalFor(pPropertySet1->testBoolProperty), Falcor::Exception);
    EXPECT_THROW(pPropertySet2->setUseGlobalFor(pPropertySet1->testFloatProperty, true), Falcor::Exception);
    EXPECT_THROW(pPropertySet2->switchUseGlobalFor(pPropertySet1->testIntProperty), Falcor::Exception);
}

} // namespace GraphEx::Test

GRAPHEX_REGISTER_SERIALIZABLE(GraphEx::Test::TestPropertySet)
