#include "AppSettingsConfig.h"
#include "LocalizationManager.h"
#include "LocalizationRemoteFetcher.h"
#include "Logger.h"
#include "PathUtils.h"
#include "StringUtils.h"
#include "Version.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4458) // declaration of 'file_name' hides class member
#pragma warning(disable: 4267) //'return' : conversion from 'size_t' to 'int', possible loss of data
#endif
#include "csv.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <locale>
#include <codecvt>

#include <easy/profiler.h>

#ifdef _WIN32
#include "windows.h"
#endif // _WIN32

static const std::string kDefaultLanguage= std::string("ja");

LocalizationManager::LocalizationManager()
	: m_currentLanguageCode("")
	, m_currentLanguage(nullptr)
{
}

LocalizationManager::~LocalizationManager()
{
	shutdown();
}

bool LocalizationManager::startup(AppSettingsConfigPtr appSettings)
{
	EASY_FUNCTION();

	m_appSettings = appSettings;

	reloadLangages();

	// Try to set the language in order: user setting, system language, default language
	if (!setLanguage(appSettings->getAppLanguage()))
	{
		if (!setLanguage(getSystemLanguage()))
		{
			if (!setLanguage(getDefaultLanguage()))
			{
				return false;
			}
		}
	}

	startRemoteFetch();

	return true;
}

void LocalizationManager::shutdown()
{
	if (m_remoteFetcher)
	{
		m_remoteFetcher->cancelFetch();
		m_remoteFetcher.reset();
	}

	unloadLanguages();
}

const std::string& LocalizationManager::getDefaultLanguage() const
{
	return kDefaultLanguage;
}

t_language_tags LocalizationManager::getSystemLanguage() const
{
	std::string localeName;

#ifdef WIN32
	LCID lcid = GetThreadLocale();
	wchar_t wszLocaleName[LOCALE_NAME_MAX_LENGTH];
	if (LCIDToLocaleName(lcid, wszLocaleName, LOCALE_NAME_MAX_LENGTH, 0) != 0)
	{
		char szLocaleName[LOCALE_NAME_MAX_LENGTH];
		StringUtils::convertWcsToMbs(wszLocaleName, szLocaleName, sizeof(szLocaleName));

		localeName= szLocaleName;
	}
#else
	localeName = std::locale("").name();
#endif

	std::vector<std::string> result;
	if (localeName == "*" || localeName.length() == 0)
	{
		result.push_back(getDefaultLanguage());
	}
	else
	{
		std::vector<std::string> localeParts= StringUtils::splitString(localeName, '.');
		std::string language= localeParts[0];

		result= StringUtils::splitString(language, '-');
	}

	return result;
}

void LocalizationManager::loadCSVsFromDirectory(
	const std::filesystem::path& locFolderPath,
	bool allowOverwrite)
{
	const std::vector<std::string> locFiles= PathUtils::listFilenamesInDirectory(locFolderPath, ".csv");

	for (auto baseFileName : locFiles)
	{
		const std::filesystem::path locFilePath = locFolderPath / baseFileName;
		std::string baseFileNameNoExt = std::filesystem::path(baseFileName).stem().string();
		std::vector<std::string> parts= StringUtils::splitString(baseFileNameNoExt, '_');

		if (parts.size() == 2)
		{
			const std::string& tableName= parts[0];
			const std::string& langCode = parts[1];

			// Fetch or create the language
			Language* language= nullptr;
			auto langIt= m_languages.find(langCode);
			if (langIt != m_languages.end())
			{
				language= langIt->second;
			}
			else
			{
				language = new Language;
				m_languages.insert({ langCode, language });
			}

			// Fetch or create the string table
			StringTable* stringTable= nullptr;
			auto tableIt= language->stringTables.find(tableName);
			if (tableIt != language->stringTables.end())
			{
				stringTable= tableIt->second;
			}
			else
			{
				stringTable = new StringTable;
				language->stringTables.insert({ tableName, stringTable });
			}

			io::CSVReader<2> in(locFilePath.string());
			in.read_header(io::ignore_extra_column, "key", "text");

			std::string key; char* utf8Text;
			while (in.read_row(key, utf8Text))
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
				std::wstring myunicodestr = convert.from_bytes(utf8Text);
				StringEntry entry= { utf8Text, myunicodestr };

				auto keyIt= stringTable->keyToTextMap.find(key);
				if (keyIt == stringTable->keyToTextMap.end())
				{
					stringTable->keyToTextMap.insert({ key, entry });
				}
				else if (allowOverwrite)
				{
					stringTable->keyToTextMap.erase(keyIt);
					stringTable->keyToTextMap.insert({ key, entry });
				}
				else
				{
					MIKAN_LOG_WARNING("LocalizationManager::loadCSVsFromDirectory") <<
						"Duplicate key \'" << key << "\' in table \'" << tableName << "\'";
				}
			}
		}
		else
		{
			MIKAN_LOG_WARNING("LocalizationManager::loadCSVsFromDirectory") << "Malformed loc filename: " << locFilePath;
		}
	}
}

void LocalizationManager::reloadLangages()
{
	unloadLanguages();

	// Load bundled strings shipped with the app
	const std::filesystem::path bundledLocPath = PathUtils::getResourceDirectory() / std::string("localization");
	loadCSVsFromDirectory(bundledLocPath, /*allowOverwrite=*/false);

	// Overlay cached strings fetched from the remote CDN (community translations)
	const std::filesystem::path cacheLocPath = getUserLocalizationCacheDir();
	if (std::filesystem::exists(cacheLocPath))
	{
		loadCSVsFromDirectory(cacheLocPath, /*allowOverwrite=*/true);
	}
}

void LocalizationManager::unloadLanguages()
{
	for (auto langIt = m_languages.begin(); langIt != m_languages.end(); )
	{
		Language* language = langIt->second;

		for (auto tableIt = language->stringTables.begin(); tableIt != language->stringTables.end(); )
		{
			StringTable* stringTable = tableIt->second;
			delete stringTable;

			tableIt = language->stringTables.erase(tableIt);
		}

		delete language;
		langIt = m_languages.erase(langIt);
	}
}

std::filesystem::path LocalizationManager::getUserLocalizationCacheDir() const
{
	return PathUtils::getHomeDirectory() / "MikanXR" / "localization";
}

void LocalizationManager::startRemoteFetch()
{
	// Use @main so community translation updates reach users without requiring a new build.
	// Version-pinned tags (e.g. @v1.0.0) are permanently cached by jsDelivr and cannot be
	// updated after tagging, which would defeat the purpose of remote localization.
	const std::string baseUrl =
		"https://cdn.jsdelivr.net/gh/brendanwalker/MikanXR@main/resources/localization";

	m_remoteFetcher = std::make_unique<LocalizationRemoteFetcher>(baseUrl, getUserLocalizationCacheDir());
	m_remoteFetcher->startFetch();
}

bool LocalizationManager::isLanguageSupported(const char* langCode) const
{
	return m_languages.find(langCode) != m_languages.end();
}

std::vector<std::string> LocalizationManager::getSupportedLanguages() const
{
	std::vector<std::string> langCodes;

	for (auto const& iter : m_languages)
		langCodes.push_back(iter.first);

	return langCodes;
}

bool LocalizationManager::setLanguage(const t_language_tags& langCodes)
{
	t_language_tags langCodeAttempt= langCodes;

	while (langCodeAttempt.size() > 0)
	{
		std::string languageId= StringUtils::joinString(langCodeAttempt, '-');
		if (setLanguage(languageId))
		{
			return true;
		}
		else
		{
			langCodeAttempt.pop_back();
		}
	}

	return false;
}

bool LocalizationManager::setLanguage(const std::string& languageId)
{
	auto langIt= m_languages.find(languageId);
	if (langIt != m_languages.end())
	{
		m_currentLanguage= langIt->second;
		m_currentLanguageCode = languageId;

		// Update the app settings with the new language.
		// Any other systems that need to know about the language change
		// can listen for changes to the app settings now that it has been updated.
		m_appSettings.lock()->setAppLanguage(languageId);

		return true;
	}

	return false;
}

const char* LocalizationManager::fetchUTF8Text(
	const char* tableName,
	const char* stringKey,
	bool* outHasString)
{
	const char* actualTableName = tableName;
	if (tableName == nullptr || tableName[0] == '\0')
	{
		actualTableName = "default";
	}

	bool bHasString= false;
	const char* result= nullptr;

	if (m_currentLanguage != nullptr)
	{
		auto tableIt = m_currentLanguage->stringTables.find(actualTableName);
		if (tableIt != m_currentLanguage->stringTables.end())
		{
			StringTable* stringTable = tableIt->second;

			auto textIt = stringTable->keyToTextMap.find(stringKey);
			if (textIt != stringTable->keyToTextMap.end())
			{
				result = textIt->second.utf8Text.c_str();
				bHasString= true;
			}
			else
			{
				result = "<INVALID STRING KEY>";
			}
		}
		else
		{
			result = "<INVALID TABLE>";
		}
	}
	else
	{
		result= "<INVALID LANGUAGE>";
	}

	if (outHasString != nullptr)
	{
		*outHasString= bHasString;
	}

	return result;
}

const wchar_t* LocalizationManager::fetchUTF16Text(
	const char* tableName,
	const char* stringKey,
	bool* outHasString)
{
	const char* actualTableName= tableName;
	if (tableName == nullptr || tableName[0] == '\0')
	{
		actualTableName= "default";
	}

	bool bHasString = false;
	const wchar_t* result = nullptr;

	if (m_currentLanguage != nullptr)
	{
		auto tableIt= m_currentLanguage->stringTables.find(actualTableName);
		if (tableIt != m_currentLanguage->stringTables.end())
		{
			StringTable* stringTable= tableIt->second;

			auto textIt= stringTable->keyToTextMap.find(stringKey);
			if (textIt != stringTable->keyToTextMap.end())
			{
				result = textIt->second.utf16Text.c_str();
				bHasString = true;
			}
			else
			{
				result = L"<INVALID STRING KEY>";
			}
		}
		else
		{
			result = L"<INVALID TABLE>";
		}
	}
	else
	{
		result = L"<INVALID LANGUAGE>";
	}

	if (outHasString != nullptr)
	{
		*outHasString = bHasString;
	}

	return result;
}
