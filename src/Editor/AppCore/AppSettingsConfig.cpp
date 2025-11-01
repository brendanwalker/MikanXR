// -- includes -----
#include "AppSettingsConfig.h"

// -- Profile Config
const std::string AppSettingsConfig::k_lastProjectPathPropertyId = "lastProjectFilePath";
const std::string AppSettingsConfig::k_appLanguagePropertyId = "appLanguage";

AppSettingsConfig::AppSettingsConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase)
{
};

configuru::Config AppSettingsConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt[k_lastProjectPathPropertyId]= m_lastProjectPath.string();
	pt[k_appLanguagePropertyId] = m_appLanguage;

	return pt;
}

void AppSettingsConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_lastProjectPath = pt.get_or<std::string>(k_lastProjectPathPropertyId, m_lastProjectPath.string());
	m_appLanguage = pt.get_or<std::string>(k_appLanguagePropertyId, m_appLanguage);
}


void AppSettingsConfig::setLastProjectPath(const std::filesystem::path& projectPath)
{
	if (m_lastProjectPath != projectPath)
	{
		m_lastProjectPath = projectPath;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_lastProjectPathPropertyId));
	}
}

void AppSettingsConfig::setAppLanguage(const std::string& appLanguage)
{
	if (m_appLanguage != appLanguage)
	{
		m_appLanguage = appLanguage;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_appLanguagePropertyId));
	}
}