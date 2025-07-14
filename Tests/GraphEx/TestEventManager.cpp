#include "GraphExTests.h"


using namespace GraphEx;


namespace GraphEx::Test
{

struct DummyEventWithParamsAndReturn : Event<int(int, int)>
{
    static bool processResult(const int& result)
    {
        return result >= 0;
    }
};

struct DummyEventWithParamsNoReturn : Event<void(int, int)> {};

struct DummyEventNoParamsWithReturn : Event<int()>
{
    static bool processResult(const int& result)
    {
        return result != 0;
    }
};

struct DummyEventNoParamsNoReturn : Event<void()> {};


TEST(EventManager, RegisterEventAndHandler)
{
    EventManager& manager = EventManager::get();
    EXPECT_NO_THROW(manager.registerEvent<DummyEventWithParamsAndReturn>());
    EXPECT_NO_THROW(manager.registerEventHandler<DummyEventWithParamsAndReturn>([](const int a, const int b) { return a + b; }));
    cleanup();
}


TEST(EventManager, RegisterHandlerForUnregisteredEvent)
{
    EventManager& manager = EventManager::get();
    EXPECT_THROW(manager.registerEventHandler<DummyEventWithParamsAndReturn>([](const int a, const int b) { return a + b; }),
                 Falcor::Exception);
    cleanup();
}


TEST(EventManager, DispatchEventWithParamsAndReturn)
{
    EventManager& manager = EventManager::get();
    manager.registerEvent<DummyEventWithParamsAndReturn>();

    auto result = 0;
    manager.registerEventHandler<DummyEventWithParamsAndReturn>([&result](const int a, const int b) { result = a + b; return result; });

    EXPECT_NO_THROW(manager.dispatchEvent<DummyEventWithParamsAndReturn>(3, 4));
    EXPECT_EQ(result, 7);
    cleanup();
}


TEST(EventManager, DispatchEventWithParamsNoReturn)
{
    EventManager& manager = EventManager::get();
    manager.registerEvent<DummyEventWithParamsNoReturn>();

    auto sum = 0;
    manager.registerEventHandler<DummyEventWithParamsNoReturn>([&sum](const int a, const int b) { sum = a + b; });

    EXPECT_NO_THROW(manager.dispatchEvent<DummyEventWithParamsNoReturn>(5, 6));
    EXPECT_EQ(sum, 11);
    cleanup();
}


TEST(EventManager, DispatchEventNoParamsWithReturn)
{
    EventManager& manager = EventManager::get();
    manager.registerEvent<DummyEventNoParamsWithReturn>();

    auto result = 0;
    manager.registerEventHandler<DummyEventNoParamsWithReturn>([&result] { result = 42; return result; });

    EXPECT_NO_THROW(manager.dispatchEvent<DummyEventNoParamsWithReturn>());
    EXPECT_EQ(result, 42);
    cleanup();
}


TEST(EventManager, EnqueueAndHandleMultipleEventTypes)
{
    EventManager& manager = EventManager::get();

    manager.registerEvent<DummyEventWithParamsAndReturn>();
    manager.registerEvent<DummyEventNoParamsWithReturn>();

    auto resultWithParams = 0;
    auto resultNoParams = 0;

    manager.registerEventHandler<DummyEventWithParamsAndReturn>([&resultWithParams](const int a, const int b)
    {
        resultWithParams = a + b; return resultWithParams;
    });

    manager.registerEventHandler<DummyEventNoParamsWithReturn>([&resultNoParams] { resultNoParams = 42; return resultNoParams; });

    EXPECT_NO_THROW(manager.enqueueEvent<DummyEventWithParamsAndReturn>(3, 4));
    EXPECT_NO_THROW(manager.enqueueEvent<DummyEventNoParamsWithReturn>());

    manager.handleEnqueuedEvents();

    EXPECT_EQ(resultWithParams, 7);
    EXPECT_EQ(resultNoParams, 42);
    cleanup();
}


TEST(EventManager, DispatchEventNoParamsNoReturn)
{
    EventManager& manager = EventManager::get();
    manager.registerEvent<DummyEventNoParamsNoReturn>();

    auto called = false;
    manager.registerEventHandler<DummyEventNoParamsNoReturn>([&called] { called = true; });

    EXPECT_NO_THROW(manager.dispatchEvent<DummyEventNoParamsNoReturn>());
    EXPECT_TRUE(called);
    cleanup();
}


TEST(EventManager, EnqueueAndHandleEvent)
{
    EventManager& manager = EventManager::get();
    manager.registerEvent<DummyEventWithParamsAndReturn>();

    auto result = 0;
    manager.registerEventHandler<DummyEventWithParamsAndReturn>([&result](const int a, const int b) { result = a + b; return result; });

    EXPECT_NO_THROW(manager.enqueueEvent<DummyEventWithParamsAndReturn>(2, 3));
    EXPECT_NO_THROW(manager.enqueueEvent<DummyEventWithParamsAndReturn>(4, 5));

    manager.handleEnqueuedEvents();
    EXPECT_EQ(result, 9);
    cleanup();
}


TEST(EventManager, HandleEventWithNoHandlers)
{
    EventManager& manager = EventManager::get();
    manager.registerEvent<DummyEventWithParamsAndReturn>();
    EXPECT_NO_THROW(manager.enqueueEvent<DummyEventWithParamsAndReturn>(1, 2));
    EXPECT_NO_THROW(manager.handleEnqueuedEvents());
    cleanup();
}


TEST(EventManager, RegisterMultipleHandlersForEvent)
{
    EventManager& manager = EventManager::get();
    manager.registerEvent<DummyEventWithParamsAndReturn>();

    auto result1 = 0;
    auto result2 = 0;
    manager.registerEventHandler<DummyEventWithParamsAndReturn>([&result1](const int a, const int b) { result1 = a + b; return result1; });
    manager.registerEventHandler<DummyEventWithParamsAndReturn>([&result2](const int a, const int b) { result2 = a * b; return result2; });

    EXPECT_NO_THROW(manager.dispatchEvent<DummyEventWithParamsAndReturn>(3, 4));
    EXPECT_EQ(result1, 7);
    EXPECT_EQ(result2, 12);
    cleanup();
}


TEST(EventManager, DispatchEventWithHandlerThrowingException)
{
    EventManager& manager = EventManager::get();
    manager.registerEvent<DummyEventWithParamsAndReturn>();
    manager.registerEventHandler<DummyEventWithParamsAndReturn>([](const int, const int) { FALCOR_THROW("Handler exception"); return 0; });
    EXPECT_THROW(manager.dispatchEvent<DummyEventWithParamsAndReturn>(1, 2), Falcor::Exception);
    cleanup();
}

} // namespace GraphEx::Test
