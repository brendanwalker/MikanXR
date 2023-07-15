#pragma once

//-- includes -----
#include <RmlUi/Core/SystemInterface.h>

#include <memory>
#include <string>
#include <map>
#include <vector>

namespace Rml
{
	class Context;

	namespace Mikan
	{
		struct EnumValue
		{
			std::string enum_string_value;
			int enum_int_value;
		};
		using EnumValuePtr= std::shared_ptr<EnumValue>;
		using EnumValueConstPtr= std::shared_ptr<const EnumValue>;

		struct EnumDefinition
		{
			std::string enum_name;
			std::vector<EnumValueConstPtr> enum_values;
		};
		using EnumDefinitionPtr= std::shared_ptr<EnumDefinition>;
		using EnumDefinitionConstPtr= std::shared_ptr<const EnumDefinition>;
	}
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

	// Enum Reflection
	bool addEnumDefinition(Rml::Mikan::EnumDefinitionConstPtr enumDefinition);
	Rml::Mikan::EnumDefinitionConstPtr getEnumDefinition(const std::string& enumName);

private:
	void registerCommonDataModelTypes();
	

	class App* m_app= nullptr;

	// Rml UI Event processor
	class RmlMikanEventInstancer* m_rmlEventInstancer = nullptr;

	// Rml UI Context
	Rml::Context* m_rmlUIContext = nullptr;
	std::map<std::string, Rml::Mikan::EnumDefinitionConstPtr> m_enumDefinitions;

	static RmlManager* m_instance;
};