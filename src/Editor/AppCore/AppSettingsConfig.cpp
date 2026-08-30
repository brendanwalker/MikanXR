// -- includes -----
#include "AppSettingsConfig.h"

// -- Profile Config
const std::string AppSettingsConfig::k_lastProjectPathPropertyId= "lastProjectFilePath";
const std::string AppSettingsConfig::k_appLanguagePropertyId= "appLanguage";
const std::string AppSettingsConfig::k_scriptEditorCommandPropertyId= "scriptEditorCommand";
const std::string AppSettingsConfig::k_httpServerPortPropertyId= "httpServerPort";
const std::string AppSettingsConfig::k_automationServerPortPropertyId= "automationServerPort";
const std::string AppSettingsConfig::k_spoutLogEnabledPropertyId= "spoutLogEnabled";
const std::string AppSettingsConfig::k_arkitDebugChannelEnabledPropertyId= "arkitDebugChannelEnabled";
const std::string AppSettingsConfig::k_arkitDebugChannelPortPropertyId= "arkitDebugChannelPort";

AppSettingsConfig::AppSettingsConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase) {};

configuru::Config AppSettingsConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt[k_lastProjectPathPropertyId]= m_lastProjectPath.string();
	pt[k_appLanguagePropertyId]= m_appLanguage;
	pt[k_scriptEditorCommandPropertyId]= m_scriptEditorCommand;
	pt[k_httpServerPortPropertyId]= m_httpServerPort;
	pt[k_automationServerPortPropertyId]= m_automationServerPort;
	pt[k_spoutLogEnabledPropertyId]= m_bSpoutLogEnabled;
	pt[k_arkitDebugChannelEnabledPropertyId]= m_bARKitDebugChannelEnabled;
	pt[k_arkitDebugChannelPortPropertyId]= m_arkitDebugChannelPort;

	return pt;
}

void AppSettingsConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_lastProjectPath= pt.get_or<std::string>(k_lastProjectPathPropertyId, m_lastProjectPath.string());
	m_appLanguage= pt.get_or<std::string>(k_appLanguagePropertyId, m_appLanguage);
	m_scriptEditorCommand= pt.get_or<std::string>(k_scriptEditorCommandPropertyId, m_scriptEditorCommand);
	m_httpServerPort= pt.get_or<int>(k_httpServerPortPropertyId, m_httpServerPort);
	m_automationServerPort= pt.get_or<int>(k_automationServerPortPropertyId, m_automationServerPort);
	m_bSpoutLogEnabled= pt.get_or<bool>(k_spoutLogEnabledPropertyId, m_bSpoutLogEnabled);
	m_bARKitDebugChannelEnabled= pt.get_or<bool>(k_arkitDebugChannelEnabledPropertyId, m_bARKitDebugChannelEnabled);
	m_arkitDebugChannelPort= pt.get_or<int>(k_arkitDebugChannelPortPropertyId, m_arkitDebugChannelPort);
}

void AppSettingsConfig::setLastProjectPath(const std::filesystem::path& projectPath)
{
	if (m_lastProjectPath != projectPath)
	{
		m_lastProjectPath= projectPath;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_lastProjectPathPropertyId));
	}
}

void AppSettingsConfig::setAppLanguage(const std::string& appLanguage)
{
	if (m_appLanguage != appLanguage)
	{
		m_appLanguage= appLanguage;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_appLanguagePropertyId));
	}
}

void AppSettingsConfig::setScriptEditorCommand(const std::string& command)
{
	if (m_scriptEditorCommand != command)
	{
		m_scriptEditorCommand= command;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_scriptEditorCommandPropertyId));
	}
}

void AppSettingsConfig::setHttpServerPort(int port)
{
	if (m_httpServerPort != port)
	{
		m_httpServerPort= port;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_httpServerPortPropertyId));
	}
}

void AppSettingsConfig::setAutomationServerPort(int port)
{
	if (m_automationServerPort != port)
	{
		m_automationServerPort= port;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_automationServerPortPropertyId));
	}
}

void AppSettingsConfig::setSpoutLogEnabled(bool bEnabled)
{
	if (m_bSpoutLogEnabled != bEnabled)
	{
		m_bSpoutLogEnabled= bEnabled;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_spoutLogEnabledPropertyId));
	}
}

void AppSettingsConfig::setARKitDebugChannelEnabled(bool bEnabled)
{
	if (m_bARKitDebugChannelEnabled != bEnabled)
	{
		m_bARKitDebugChannelEnabled= bEnabled;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_arkitDebugChannelEnabledPropertyId));
	}
}

void AppSettingsConfig::setARKitDebugChannelPort(int port)
{
	if (m_arkitDebugChannelPort != port)
	{
		m_arkitDebugChannelPort= port;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_arkitDebugChannelPortPropertyId));
	}
}
