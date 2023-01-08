#pragma once

//-- includes -----
#include <RmlUi/Core/SystemInterface.h>

namespace Rml
{
	class Context;
};

//-- definitions -----
class RmlManager : public Rml::SystemInterface
{
public:
	RmlManager(class App* app);
	~RmlManager();

	bool preRendererStartup();
	bool postRendererStartup();
	void update();
	void shutdown();

	// Rml::SystemInterface
	virtual double GetElapsedTime() override;
	virtual int TranslateString(Rml::String& translated, const Rml::String& input) override;
	virtual bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
	virtual void SetMouseCursor(const Rml::String& cursor_name) override;
	virtual void SetClipboardText(const Rml::String& text) override;
	virtual void GetClipboardText(Rml::String& text) override;

	static RmlManager* getInstance()
	{
		return m_instance;
	}

	inline Rml::Context* getRmlUIContext() const { return m_rmlUIContext; }

private:
	void registerCommonDataModelTypes();

	class App* m_app= nullptr;

	// Rml UI Event processor
	class RmlMikanEventInstancer* m_rmlEventInstancer = nullptr;

	// Rml UI Context
	Rml::Context* m_rmlUIContext = nullptr;

	static RmlManager* m_instance;
};