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
	class DataModelConstructor;

	namespace Mikan
	{
		struct EnumDefinition
		{
			std::string enum_name;
			std::vector<std::string> enum_string_values;
			std::vector<int> enum_int_values;
		};
		using EnumDefinitionPtr= std::shared_ptr<EnumDefinition>;
		using EnumDefinitionConstPtr= std::shared_ptr<const EnumDefinition>;
	}
};

//-- definitions -----
class RmlManager : public Rml::SystemInterface
{
public:
	RmlManager(class MainWindow* ownerWindow);
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

	Rml::Context* pushContext(const std::string& contextName);
	Rml::Context* getRmlUIContext() const;
	void popContext(Rml::Context* context);

	inline class MainWindow* getOwnerWindow() const { return m_ownerWindow; }

	// Enum Reflection
	bool addEnumDefinition(Rml::Mikan::EnumDefinitionPtr enumDefinition);
	Rml::Mikan::EnumDefinitionConstPtr getEnumDefinition(const std::string& enumName);
	void bindEnumDefinitionsToDataModel(Rml::DataModelConstructor& constructor);

private:
	void registerCommonDataModelTypes(Rml::Context* context);
	
	class MainWindow* m_ownerWindow= nullptr;

	// Rml UI Event processor
	class RmlMikanEventInstancer* m_rmlEventInstancer = nullptr;
	std::shared_ptr<struct RmlMikanDecoratorInstancers> m_rmlDecoratorInstancers;

	// Rml UI Context
	std::vector<Rml::Context*> m_rmlUIContextStack;
	std::map<std::string, Rml::Mikan::EnumDefinitionPtr> m_enumDefinitions;

	static RmlManager* m_instance;
};