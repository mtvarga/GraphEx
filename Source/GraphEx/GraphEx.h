#pragma once

#include "Application.h"

#include "API/Event.h"
#include "API/EventManager.h"
#include "API/Module.h"
#include "API/ModuleRegistry.h"

#include "Core/CameraManager.h"
#include "Core/CoreEvents.h"
#include "Core/CoreTypes.h"
#include "Core/RenderManager.h"
#include "Core/RenderModule.h"
#include "Core/SceneManager.h"

#include "Serialization/Serialization.h"
#include "Serialization/SerializableTypeRegistry.h"

#include "UI/UI.h"
#include "UI/UIHelpers.h"

#include "Utils/DispatchManager.h"
#include "Utils/GlobalLocalProperty.h"
#include "Utils/ProgramContext.h"
#include "Utils/ProgramWrapper.h"
#include "Utils/Standard.h"


namespace GraphEx
{

GRAPHEX_EXPORTABLE std::filesystem::path getRuntimeDataDirectory();

} // namespace GraphEx