// -- includes -----
#include "AppSettingsConfig.h"

// -- Profile Config
const std::string AppSettingsConfig::k_lastProjectPathPropertyId = "lastProjectFilePath";
const std::string AppSettingsConfig::k_appLanguagePropertyId = "appLanguage";
const std::string AppSettingsConfig::k_scriptEditorCommandPropertyId = "scriptEditorCommand";

AppSettingsConfig::AppSettingsConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase)
{
};

configuru::Config AppSettingsConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt[k_lastProjectPathPropertyId]= m_lastProjectPath.string();
	pt[k_appLanguagePropertyId] = m_appLanguage;
	pt[k_scriptEditorCommandPropertyId] = m_scriptEditorCommand;

	return pt;
}

void AppSettingsConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_lastProjectPath = pt.get_or<std::string>(k_lastProjectPathPropertyId, m_lastProjectPath.string());
	m_appLanguage = pt.get_or<std::string>(k_appLanguagePropertyId, m_appLanguage);
	m_scriptEditorCommand = pt.get_or<std::string>(k_scriptEditorCommandPropertyId, m_scriptEditorCommand);
}


void AppSettingsConfig::setLastProjectPath(const std::filesystem::path& projectPath)
{
	if (m_lastProjectPath != projectPath)
	{
		m_lastProjectPath = projectPath;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_lastProjectPathPropertyId));
	}
}

void AppSettingsConfig::setAppLanguage(const std::string& appLanguage)
{
	if (m_appLanguage != appLanguage)
	{
		m_appLanguage = appLanguage;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_appLanguagePropertyId));
	}
}

void AppSettingsConfig::setScriptEditorCommand(const std::string& command)
{
	if (m_scriptEditorCommand != command)
	{
		m_scriptEditorCommand = command;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_scriptEditorCommandPropertyId));
	}
}