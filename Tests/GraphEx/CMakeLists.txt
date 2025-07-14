target_sources(${GRAPHEX_TESTS_TARGET_NAME} PRIVATE
    GraphExTests.h
    GraphExTests.cpp

    TestApplication.cpp
    TestEventManager.cpp
    TestDispatchManager.cpp
    TestGlobalLocalProperty.cpp
    TestModuleRegistry.cpp
    TestModuleContainer.cpp
    TestModuleDependencies.cpp
    TestModuleSerialization.cpp
)

target_compile_definitions(${GRAPHEX_TESTS_TARGET_NAME} PRIVATE
    GRAPHEX_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

graphex_target_source_group(${GRAPHEX_TESTS_TARGET_NAME} "Tests")
