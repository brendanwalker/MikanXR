#include "RmlManager.h"

#include "App.h"
#include "AppStage.h"
#include "Logger.h"
#include "MikanClientTypes.h"
#include "ProfileConfig.h"
#include "PathUtils.h"
#include "Renderer.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/EventListenerInstancer.h>
// Mikan extensions for RmlUI
#include "RmlMikanPlugin.h"

#include "SDL_clipboard.h"
#include "SDL_timer.h"

//-- private classes -----
class RmlMikanEventListener : public Rml::EventListener
{
public:
	RmlMikanEventListener(
		App* app,
		const Rml::String& value,
		Rml::Element* element)
		: m_app(app)
		, m_value(value)
		, m_element(element)
	{

	}

	virtual void ProcessEvent(Rml::Event& event) override
	{
		AppStage* appStage = m_app->getCurrentAppStage();
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
	App* m_app = nullptr;
	Rml::String m_value;
	Rml::Element* m_element;
};

class RmlMikanEventInstancer : public Rml::EventListenerInstancer
{
public:
	RmlMikanEventInstancer(App* app)
		: m_app(app)
	{

	}
	Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override
	{
		return new RmlMikanEventListener(m_app, value, element);
	}

private:
	App* m_app = nullptr;
};

class RmlMikanFileInterface : public Rml::FileInterface
{
public:
	RmlMikanFileInterface() {}
	virtual ~RmlMikanFileInterface() {}

	/// Opens a file.		
	Rml::FileHandle Open(const Rml::String& path) override
	{
		if (PathUtils::isAbsolutePath(path))
		{
			// Attempt to open the absolute file relative 
			FILE* fp = fopen(path.c_str(), "rb");
			return (Rml::FileHandle)fp;
		}
		else
		{
			std::string absPath = PathUtils::makeAbsoluteResourceFilePath(path);

			// Attempt to open the file relative to the application's root.
			FILE* fp = fopen(absPath.c_str(), "rb");
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
RmlManager::RmlManager(App* app)
	: m_app(app)
	, m_rmlEventInstancer(new RmlMikanEventInstancer(app))
{

}

RmlManager::~RmlManager()
{
	delete m_rmlEventInstancer;
}

bool RmlManager::preRendererStartup()
{
	// Tell the UI libary this class implements the RML System Interface
	Rml::SetSystemInterface(this);
	Rml::SetFileInterface(new RmlMikanFileInterface());
	Rml::Mikan::Initialise();
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

		Renderer* renderer= m_app->getRenderer();
		int window_width = renderer->getSDLWindowWidth();
		int window_height = renderer->getSDLWindowHeight();
		m_rmlUIContext = Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));
		Rml::Debugger::Initialise(m_rmlUIContext);

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

void RmlManager::registerCommonDataModelTypes()
{
	Rml::DataModelConstructor constructor = m_rmlUIContext->CreateDataModel("data_model_globals");

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

	// Transform function for converting anchor id to anchor name
	constructor.RegisterTransformFunc(
		"to_anchor_name",
		[this](Rml::Variant& variant, const Rml::VariantList& /*arguments*/) -> bool {
			const MikanSpatialAnchorID anchorId = variant.Get<int>(-1);

			MikanSpatialAnchorInfo anchorInfo;
			if (m_app->getProfileConfig()->getSpatialAnchorInfo(anchorId, anchorInfo))
			{
				variant = Rml::String(anchorInfo.anchor_name);
				return true;
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
}

// Rml::SystemInterface
double RmlManager::GetElapsedTime()
{
	return double(SDL_GetTicks()) / 1000.0;
}

int RmlManager::TranslateString(Rml::String& translated, const Rml::String& input)
{
	translated = input;

	AppStage* appStage = m_app->getCurrentAppStage();
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
	m_app->getRenderer()->setSDLMouseCursor(cursor_name);
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