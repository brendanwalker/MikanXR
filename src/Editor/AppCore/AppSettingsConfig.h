#pragma once

// -- includes -----
#include "CommonConfig.h"

#include <filesystem>

// -- definitions -----
class AppSettingsConfig : public CommonConfig
{
public:
	AppSettingsConfig(const std::string& fnamebase = "AppSettingsConfig");

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	static const std::string k_lastProjectPathPropertyId;
	inline bool hasLastProjectPath() const { return !m_lastProjectPath.empty(); }
	inline const std::filesystem::path& getLastProjectPath() const { return m_lastProjectPath; }
	void setLastProjectPath(const std::filesystem::path& projectPath);

	static const std::string k_appLanguagePropertyId;
	inline const std::string& getAppLanguage() const { return m_appLanguage; }
	void setAppLanguage(const std::string& appLanguage);

protected:
	std::filesystem::path m_lastProjectPath;
	std::string m_appLanguage;
};