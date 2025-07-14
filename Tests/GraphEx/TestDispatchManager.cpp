#include "GraphExTests.h"


using namespace GraphEx;


namespace GraphEx::Test
{

TEST(DispatchManager, DispatchWithParamsAndReturnValue)
{
    auto resultCallbackCalled = false;

    DispatchManager<int, std::tuple<int, int>> manager([&resultCallbackCalled](const int& result)
    {
        resultCallbackCalled = true;
        EXPECT_EQ(result, 7);
        return true;
    });

    const std::function target = [](const int a, const int b) { return a + b; };
    manager.registerTarget(InvokerForDispatchTarget(target));
    manager.performDispatch(3, 4);

    EXPECT_TRUE(resultCallbackCalled);
}

TEST(DispatchManager, DispatchWithParamsNoReturnValue)
{
    DispatchManager<void, std::tuple<int, int>> manager;

    auto sum = 0;

    const std::function target = [&sum](const int a, const int b) { sum = a + b; };
    manager.registerTarget(InvokerForDispatchTarget(target));
    manager.performDispatch(5, 6);

    EXPECT_EQ(sum, 11);
}


TEST(DispatchManager, DispatchNoParamsWithReturnValue)
{
    auto resultCallbackCalled = false;

    DispatchManager<int, std::tuple<>> manager([&resultCallbackCalled](const int& result)
    {
        resultCallbackCalled = true;
        EXPECT_EQ(result, 42);
        return true;
    });

    const std::function target = [] { return 42; };
    manager.registerTarget(InvokerForDispatchTarget(target));
    manager.performDispatch();

    EXPECT_TRUE(resultCallbackCalled);
}


TEST(DispatchManager, DispatchNoParamsNoReturnValue)
{
    DispatchManager<void, std::tuple<>> manager;

    auto called = false;
    const std::function target = [&called] { called = true; };
    manager.registerTarget(InvokerForDispatchTarget(target));
    manager.performDispatch();

    EXPECT_TRUE(called);
}


TEST(DispatchManager, EnqueueAndHandleDispatches)
{
    auto counter1 = 0;
    auto counter2 = 0;

    DispatchManager<int, std::tuple<int, int>> manager([&counter1](const int& result)
    {
        ++counter1;
        EXPECT_EQ(result, 10);
        return true;
    });

    const std::function target = [&counter2](const int a, const int b) { ++counter2; return a + b; };
    manager.registerTarget(InvokerForDispatchTarget(target));
    manager.enqueueDispatch(2, 8);
    manager.enqueueDispatch(3, 7);
    manager.enqueueDispatch(4, 6);
    manager.handleEnqueuedDispatches();

    EXPECT_EQ(counter1, 3);
    EXPECT_EQ(counter1, counter2);
}


TEST(DispatchManager, NoTargetsRegistered)
{
    DispatchManager<int, std::tuple<int, int>> manager;
    EXPECT_NO_THROW(manager.performDispatch(1, 2));
}


TEST(DispatchManager, MultipleTargetsRegistered)
{
    auto resultCallbackCalled = false;
    auto previousResult = 0;

    DispatchManager<int, std::tuple<int>> manager([&resultCallbackCalled, &previousResult](const int& result)
    {
        resultCallbackCalled = true;
        EXPECT_TRUE(result != previousResult);
        EXPECT_TRUE(result % 2 == 0);
        previousResult = result;
        return true;
    });

    const std::function target1 = [](const int a) { return a * 2; };
    const std::function target2 = [](const int a) { return a * 6; };
    manager.registerTarget(InvokerForDispatchTarget(target1));
    manager.registerTarget(InvokerForDispatchTarget(target2));
    manager.performDispatch(1);

    EXPECT_TRUE(resultCallbackCalled);
}


TEST(DispatchManager, DispatchResultCallbackHaltsExecution)
{
    auto counter = 0;

    DispatchManager<int, std::tuple<int, int>> manager([&counter](const int&)
    {
        ++counter;
        return false;
    });

    auto result1 = 0;
    auto result2 = 0;

    const std::function target1 = [&result1](const int a, const int b) { result1 = a + b; return result1; };
    const std::function target2 = [&result2](const int a, const int b) { result2 = a * b; return result2; };
    manager.registerTarget(InvokerForDispatchTarget(target1));
    manager.registerTarget(InvokerForDispatchTarget(target2));
    manager.performDispatch(2, 3);

    EXPECT_EQ(result1, 5);
    EXPECT_EQ(result2, 0);
    EXPECT_EQ(counter, 1);
}

} // namespace GraphEx::Test
