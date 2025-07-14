target_sources(${GRAPHEX_TARGET_NAME} PRIVATE
    Application.h
    Application.cpp
    GraphEx.h
    GraphEx.cpp

    API/Event.h
    API/EventManager.h
    API/EventManager.cpp
    API/Module.h
    API/ModuleRegistry.h
    API/ModuleRegistry.cpp

    Core/CameraManager.h
    Core/CameraManager.cpp
    Core/CoreEvents.h
    Core/CoreEvents.cpp
    Core/CoreTypes.h
    Core/CoreTypes.cpp
    Core/RenderManager.h
    Core/RenderManager.cpp
    Core/RenderModule.h
    Core/RenderModule.cpp
    Core/SceneManager.h
    Core/SceneManager.cpp

    Serialization/Internal/CameraSerialization.h
    Serialization/Internal/ModuleSerialization.h
    Serialization/Internal/VectorSerialization.h
    Serialization/Internal/SerializationMacros.h
    Serialization/Internal/SerializationManager.h
    Serialization/Internal/SerializationManager.cpp
    Serialization/Internal/SerializationTemplates.h
    Serialization/Internal/ReferenceSerialization.h
    Serialization/SerializableTypeRegistry.h
    Serialization/Serialization.h

    UI/UI.h
    UI/UI.cpp
    UI/UIHelpers.h
    UI/UIHelpers.cpp

    Utils/DispatchManager.h
    Utils/DispatchManager.cpp
    Utils/GlobalLocalProperty.h
    Utils/GlobalLocalProperty.cpp
    Utils/ProgramContext.h
    Utils/ProgramContext.cpp
    Utils/ProgramWrapper.h
    Utils/ProgramWrapper.cpp
    Utils/Standard.h
)

target_compile_definitions(${GRAPHEX_TARGET_NAME} PRIVATE
    GRAPHEX_EXPORT_EXPORTABLES
)

graphex_target_source_group(${GRAPHEX_TARGET_NAME} "")