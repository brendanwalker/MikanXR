#include "RmlManager.h"

#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "App.h"
#include "AppStage.h"
#include "CameraObjectSystem.h"
#include "CameraComponent.h"
#include "CompositorConstants.h"
#include "CompositorObjectSystem.h"
#include "CompositorComponent.h"
#include "IVRDevice.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MarkerObjectSystem.h"
#include "MarkerComponent.h"
#include "MikanSpatialAnchorTypes.h"
#include "MikanStencilTypes.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "PathUtils.h"
#include "SdlManager.h"
#include "StageObjectSystem.h"
#include "StageComponent.h"
#include "StencilComponent.h"
#include "StencilObjectSystem.h"
#include "SceneObjectSystem.h"
#include "SceneComponent.h"
#include "TrackingVolumeObjectSystem.h"
#include "TrackingVolumeComponent.h"
#include "TrackingMountObjectSystem.h"
#include "TrackingMountComponent.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"
#include "VideoSourceSystem.h"
#include "VideoSourceComponent.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/EventListenerInstancer.h>
#include <RmlUi/Core/Factory.h>

#include "DecoratorInstancerArucoMarker.h"

#include "SDL_clipboard.h"
#include "SDL_timer.h"

//-- private classes -----
class RmlMikanEventListener : public Rml::EventListener
{
public:
	RmlMikanEventListener(
		MainWindow* ownerWindow,
		const Rml::String& value,
		Rml::Element* element)
		: m_ownerWindow(ownerWindow)
		, m_value(value)
		, m_element(element)
	{

	}

	virtual void ProcessEvent(Rml::Event& event) override
	{
		AppStage* appStage = m_ownerWindow->getCurrentAppStage();
		if (appStage == nullptr)
			return;

		switch (event.GetId())
		{
			case Rml::EventId::Click:
				appStage->onRmlClickEvent(m_value);
				break;
			default:
				break;
		}
	}

private:
	MainWindow* m_ownerWindow = nullptr;
	Rml::String m_value;
	Rml::Element* m_element;
};

class RmlMikanEventInstancer : public Rml::EventListenerInstancer
{
public:
	RmlMikanEventInstancer(class MainWindow* ownerWindow)
		: m_ownerWindow(ownerWindow)
	{

	}
	Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override
	{
		return new RmlMikanEventListener(m_ownerWindow, value, element);
	}

private:
	class MainWindow* m_ownerWindow = nullptr;
};

struct RmlMikanDecoratorInstancers
{
	DecoratorInstancerArucoMarker arucoMarkerInstancer;
};

class RmlMikanFileInterface : public Rml::FileInterface
{
public:
	RmlMikanFileInterface() {}
	virtual ~RmlMikanFileInterface() {}

	/// Opens a file.		
	Rml::FileHandle Open(const Rml::String& pathString) override
	{
		const std::filesystem::path path= pathString;

		if (path.is_absolute())
		{
			// Attempt to open the absolute file relative 
			FILE* fp = fopen(pathString.c_str(), "rb");
			return (Rml::FileHandle)fp;
		}
		else
		{
			const std::filesystem::path absPath = PathUtils::makeAbsoluteResourceFilePath(path);
			const std::string absPathString = absPath.string();

			// Attempt to open the file relative to the application's root.
			FILE* fp = fopen(absPathString.c_str(), "rb");
			return (Rml::FileHandle)fp;
		}
	}

	/// Closes a previously opened file.
	void Close(Rml::FileHandle file) override
	{
		fclose((FILE*)file);
	}

	/// Reads data from a previously opened file.
	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override
	{
		return fread(buffer, 1, size, (FILE*)file);
	}

	/// Seeks to a point in a previously opened file.
	bool Seek(Rml::FileHandle file, long offset, int origin) override
	{
		return fseek((FILE*)file, offset, origin) == 0;
	}

	/// Returns the current position of the file pointer.
	size_t Tell(Rml::FileHandle file) override
	{
		return ftell((FILE*)file);
	}
};

// -- RmlManager -----
RmlManager* RmlManager::m_instance = nullptr;

RmlManager::RmlManager(MainWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
	, m_rmlEventInstancer(new RmlMikanEventInstancer(ownerWindow))
	, m_rmlDecoratorInstancers(new RmlMikanDecoratorInstancers)
{

}

RmlManager::~RmlManager()
{
	delete m_rmlEventInstancer;
	delete m_rmlDecoratorInstancers;
}

bool RmlManager::preRendererStartup()
{
	m_instance= this;

	// Tell the UI libary this class implements the RML System Interface
	Rml::SetSystemInterface(this);
	Rml::SetFileInterface(new RmlMikanFileInterface());
	Rml::Factory::RegisterEventListenerInstancer(m_rmlEventInstancer);

	return true;
}

bool RmlManager::postRendererStartup()
{
	if (Rml::Initialise())
	{
		struct FontFace
		{
			const char* filename;
			bool fallback_face;
		};
		FontFace font_faces[] = {
			{"font/LatoLatin-Regular.ttf", false},
			{"font/LatoLatin-Italic.ttf", false},
			{"font/LatoLatin-Bold.ttf", false},
			{"font/LatoLatin-BoldItalic.ttf", false},
			{"font/NotoEmoji-Regular.ttf", true},
		};

		for (const FontFace& face : font_faces)
		{
			Rml::LoadFontFace(face.filename, face.fallback_face);
		}

		int window_width = (int)m_ownerWindow->getWidth();
		int window_height = (int)m_ownerWindow->getHeight();
		m_rmlUIContext = Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));
		Rml::Debugger::Initialise(m_rmlUIContext);

		// Register custom decorators
		Rml::Factory::RegisterDecoratorInstancer("aruco-marker", &m_rmlDecoratorInstancers->arucoMarkerInstancer);

		// Register common data model types shared amongst all UI
		registerCommonDataModelTypes();

		return true;
	}
	else
	{
		MIKAN_LOG_ERROR("RmlManager::postRendererStartup") << "Failed to initialize the RmlUi";
		return false;
	}
}

bool RmlManager::addEnumDefinition(Rml::Mikan::EnumDefinitionConstPtr enumDefinition)
{
	auto it = m_enumDefinitions.find(enumDefinition->enum_name);
	if (it == m_enumDefinitions.end())
	{
		m_enumDefinitions.insert({enumDefinition->enum_name, enumDefinition});
		return true;
	}

	return false;
}

Rml::Mikan::EnumDefinitionConstPtr RmlManager::getEnumDefinition(const std::string& enumName)
{
	auto it= m_enumDefinitions.find(enumName);
	if (it != m_enumDefinitions.end())
	{
		return it->second;
	}

	return Rml::Mikan::EnumDefinitionConstPtr();
}

template <typename t_enum_class>
static void registerEnumDefinition(
	Rml::DataModelConstructor& constructor,
	const std::string& enumName, 
	const std::string* enumStrings)
{
	Rml::Mikan::EnumDefinitionPtr enumDefinition = std::make_shared<Rml::Mikan::EnumDefinition>();
	enumDefinition->enum_name = enumName;

	for (int enumIntValue = 0; enumIntValue < (int)t_enum_class::COUNT; ++enumIntValue)
	{
		enumDefinition->enum_string_values.push_back(enumStrings[enumIntValue]);
		enumDefinition->enum_int_values.push_back(enumIntValue);
	}

	constructor.Bind(enumName+"_values", &enumDefinition->enum_int_values);
	constructor.Bind(enumName+"_names", &enumDefinition->enum_string_values);
	RmlManager::getInstance()->addEnumDefinition(enumDefinition);
}

template<class t_system_type>
std::shared_ptr<t_system_type> rmlGetSystemOfType(RmlManager* rmlManager)
{
	ProjectManagerPtr objectSystemManager = rmlManager->getOwnerWindow()->getProjectManager();

	return objectSystemManager->getSystemOfType<t_system_type>();
}

void RmlManager::registerCommonDataModelTypes()
{
	Rml::DataModelConstructor constructor = m_rmlUIContext->CreateDataModel("data_model_globals");

	// Enums
	registerEnumDefinition<eStencilCullMode>(constructor, "stencil_cull_mode", k_stencilCullModeStrings);
	registerEnumDefinition<eCharucoDictionaryType>(constructor, "marker_dictionary", k_charucoDictionaryStrings);

	// String arrays
	constructor.RegisterArray<Rml::Vector<Rml::String>>();
	constructor.RegisterArray<Rml::Vector<int>>();

	// Vector2f
	if (auto struct_handle = constructor.RegisterStruct<Rml::Vector2f>())
	{
		struct_handle.RegisterMember("x", &Rml::Vector2f::x);
		struct_handle.RegisterMember("y", &Rml::Vector2f::y);
	}

	// Vector3f
	if (auto struct_handle = constructor.RegisterStruct<Rml::Vector3f>())
	{
		struct_handle.RegisterMember("x", &Rml::Vector3f::x);
		struct_handle.RegisterMember("y", &Rml::Vector3f::y);
		struct_handle.RegisterMember("z", &Rml::Vector3f::z);
	}

	// Vector4f
	if (auto struct_handle = constructor.RegisterStruct<Rml::Vector4f>())
	{
		struct_handle.RegisterMember("x", &Rml::Vector4f::x);
		struct_handle.RegisterMember("y", &Rml::Vector4f::y);
		struct_handle.RegisterMember("z", &Rml::Vector4f::z);
		struct_handle.RegisterMember("w", &Rml::Vector4f::w);
	}

	// Transform function for converting anchor id to anchor name
	RmlManager* rmlManager = this;
	constructor.RegisterTransformFunc(
		"to_anchor_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanSpatialAnchorID anchorId = variant.Get<int>(-1);

			auto anchorObjectSystem = rmlGetSystemOfType<AnchorObjectSystem>(rmlManager);
			if (anchorId != INVALID_MIKAN_ID)
			{
				auto anchorComponent = anchorObjectSystem->getSpatialAnchorById(anchorId);
				if (anchorComponent != nullptr)
				{
					variant = Rml::String(anchorComponent->getName());
					return true;
				}
			}
			else
			{
				variant = Rml::String("<None>");
				return true;
			}
			return false;
		});

	constructor.RegisterTransformFunc(
		"to_compositor_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanCompositorID compositorId = variant.Get<int>(-1);

			auto compositorObjectSystem = rmlGetSystemOfType<CompositorObjectSystem>(rmlManager);
			auto compositorComponent = compositorObjectSystem->getCompositorById(compositorId);
			if (compositorComponent != nullptr)
			{
				variant = Rml::String(compositorComponent->getName());
				return true;
			}
			return false;
		});

	constructor.RegisterTransformFunc(
		"to_marker_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanMarkerID markerId = variant.Get<int>(-1);

			auto markerObjectSystem = rmlGetSystemOfType<MarkerObjectSystem>(rmlManager);
			auto markerComponent = markerObjectSystem->getMarkerById(markerId);
			if (markerComponent != nullptr)
			{
				variant = Rml::String(markerComponent->getName());
				return true;
			}
			return false;
		});

	constructor.RegisterTransformFunc(
		"to_stage_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanStageID stageId = variant.Get<int>(-1);

			auto stageObjectSystem = rmlGetSystemOfType<StageObjectSystem>(rmlManager);
			auto stageComponent = stageObjectSystem->getStageById(stageId);
			if (stageComponent != nullptr)
			{
				variant = Rml::String(stageComponent->getName());
				return true;
			}
			return false;
		});

	constructor.RegisterTransformFunc(
		"to_scene_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanSceneID sceneId = variant.Get<int>(-1);

			auto sceneObjectSystem = rmlGetSystemOfType<SceneObjectSystem>(rmlManager);
			auto sceneComponent = sceneObjectSystem->getSceneById(sceneId);
			if (sceneComponent != nullptr)
			{
				variant = Rml::String(sceneComponent->getName());
				return true;
			}
			return false;
		});

	constructor.RegisterTransformFunc(
		"to_volume_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanTrackingVolumeID volumeId = variant.Get<int>(-1);

			auto volumeObjectSystem = rmlGetSystemOfType<TrackingVolumeObjectSystem>(rmlManager);
			auto volumeComponent = volumeObjectSystem->getTrackingVolumeById(volumeId);
			if (volumeComponent != nullptr)
			{
				variant = Rml::String(volumeComponent->getName());
				return true;
			}
			return false;
		});

	constructor.RegisterTransformFunc(
		"to_tracking_mount_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanTrackingMountID mountId = variant.Get<int>(-1);

			auto mountObjectSystem = rmlGetSystemOfType<TrackingMountObjectSystem>(rmlManager);
			auto mountComponent = mountObjectSystem->getTrackingMountById(mountId);
			if (mountComponent != nullptr)
			{
				variant = Rml::String(mountComponent->getName());
				return true;
			}
			return false;
		});

	// Transform function for converting stencil id to stencil name
	constructor.RegisterTransformFunc(
		"to_stencil_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanStencilID stencilId = variant.Get<int>(-1);

			auto stencilObjectSystem = rmlGetSystemOfType<StencilObjectSystem>(rmlManager);
			auto stencilComponent= stencilObjectSystem->getStencilById(stencilId);
			if (stencilComponent != nullptr)
			{
				variant = stencilComponent->getName();
				return true;
			}
			return false;
		});

	// Transform function for converting stencil id to stencil name
	constructor.RegisterTransformFunc(
		"to_camera_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanCameraID cameraId = variant.Get<int>(-1);

			if (cameraId == INVALID_MIKAN_ID)
			{
				variant = Rml::String("<None>");
				return true;
			}
			else
			{
				auto cameraObjectSystem = rmlGetSystemOfType<CameraObjectSystem>(rmlManager);
				auto cameraComponent = cameraObjectSystem->getCameraById(cameraId);
				if (cameraComponent != nullptr)
				{
					variant = cameraComponent->getName();
					return true;
				}
			}

			return false;
		});

	// Transform function for converting full file path to a trimmed path
	constructor.RegisterTransformFunc(
		"to_short_path",
		[](Rml::Variant& variant, const Rml::VariantList& arguments) -> bool {
			const Rml::String filePath = variant.Get<Rml::String>("");
			const size_t maxLength = arguments[0].Get<int>(20);

			variant= PathUtils::createTrimmedPathString(filePath, maxLength);

			return true;
		});

	// Transform function for converting full file path to a trimmed path
	constructor.RegisterTransformFunc(
		"to_video_source_friendly_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& arguments) -> bool {
			const MikanVideoSourceID videoSourceId = variant.Get<int>(-1);
		
			auto videoSourceSystem = rmlGetSystemOfType<VideoSourceSystem>(rmlManager);
			auto videoSourceComponent = videoSourceSystem->getVideoSourceById(videoSourceId);
			if (videoSourceComponent)
			{
				const Rml::String friendlyName = videoSourceComponent->getName();

				variant = friendlyName;
				return true;
			}

			return false;
		});

	// Transform function for converting full file path to a trimmed path
	constructor.RegisterTransformFunc(
		"to_vr_device_friendly_name",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& arguments) -> bool {
			const Rml::String devicePath = variant.Get<Rml::String>("");

			auto vrObjectSystem = rmlGetSystemOfType<VRObjectSystem>(rmlManager);
			VRDeviceComponentPtr deviceView= vrObjectSystem->getVRDeviceByPath(devicePath);
			if (deviceView)
			{
				const IVRDevice* deviceInterface= deviceView->getVRDeviceInterface();
				const std::string trackerRole= deviceInterface->getTrackerRole();
				const std::string serialNumber = deviceInterface->getSerialNumber();
				const Rml::String friendlyName = trackerRole + " - " + serialNumber;

				variant= friendlyName;
				return true;
			}

			return false;
		});

	constructor.RegisterTransformFunc(
		"to_enum_string",
		[rmlManager](Rml::Variant& variant, const Rml::VariantList& arguments) -> bool {
			const int enumIntValue = variant.Get<int>(-1);
			if (enumIntValue != -1 && arguments.size() == 1)
			{
				const std::string enumName = arguments[0].Get<Rml::String>();
				Rml::Mikan::EnumDefinitionConstPtr enumDefinition= rmlManager->getEnumDefinition(enumName);

				if (enumDefinition && enumIntValue >= 0 && enumIntValue < enumDefinition->enum_string_values.size())
				{
					Rml::String enumStringValue= enumDefinition->enum_string_values[enumIntValue];
					variant = enumStringValue;
					return true;
				}
			}
			return false;
		});
}

void RmlManager::update()
{
	if (m_rmlUIContext != nullptr)
	{
		m_rmlUIContext->Update();
	}
}

void RmlManager::shutdown()
{
	Rml::Shutdown();
	m_rmlUIContext = nullptr;
	m_instance= nullptr;
}

// Rml::SystemInterface
double RmlManager::GetElapsedTime()
{
	return double(SDL_GetTicks()) / 1000.0;
}

int RmlManager::TranslateString(Rml::String& translated, const Rml::String& input)
{
	translated = input;

	AppStage* appStage = m_ownerWindow->getCurrentAppStage();
	if (appStage != nullptr)
	{
		bool bHasString = false;
		const std::string& contextName = appStage->getAppStageName();
		const char* result = locTextUTF8(contextName.c_str(), input.c_str(), &bHasString);

		if (bHasString)
		{
			translated = result;
			return 1;
		}
	}

	return 0;
}

bool RmlManager::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
	switch (type)
	{
		case Rml::Log::LT_ASSERT:
			MIKAN_LOG_FATAL("Rml::LogMessage") << message;
			return true;
		case Rml::Log::LT_ALWAYS:
		case Rml::Log::LT_ERROR:
			MIKAN_LOG_ERROR("Rml::LogMessage") << message;
			return true;
		case Rml::Log::LT_WARNING:
			MIKAN_LOG_WARNING("Rml::LogMessage") << message;
			return true;
		case Rml::Log::LT_INFO:
			MIKAN_LOG_INFO("Rml::LogMessage") << message;
			return true;
		case Rml::Log::LT_DEBUG:
			MIKAN_LOG_DEBUG("Rml::LogMessage") << message;
			return true;
	}

	return false;
}

void RmlManager::SetMouseCursor(const Rml::String& cursor_name)
{
	App::getInstance()->getSdlManager()->setSDLMouseCursor(cursor_name);
}

void RmlManager::SetClipboardText(const Rml::String& text_utf8)
{
	SDL_SetClipboardText(text_utf8.c_str());
}

void RmlManager::GetClipboardText(Rml::String& text)
{
	char* raw_text = SDL_GetClipboardText();
	text = Rml::String(raw_text);
	SDL_free(raw_text);
}
