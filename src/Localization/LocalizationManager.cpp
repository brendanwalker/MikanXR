#include "LocalizationManager.h"
#include "Logger.h"
#include "PathUtils.h"
#include "StringUtils.h"

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

LocalizationManager* LocalizationManager::m_instance= nullptr;

LocalizationManager::LocalizationManager()
	: m_currentLanguageCode("")
	, m_currentLanguage(nullptr)
{
	m_instance= this;
}

LocalizationManager::~LocalizationManager()
{
	shutdown();

	m_instance= nullptr;
}

bool LocalizationManager::startup()
{
	EASY_FUNCTION();

	reloadLangages();

	if (!setLanguage(getSystemLanguage()))
	{
		if (!setLanguage(getDefaultLanguage()))
		{
			return false;
		}
	}

	return true;
}

void LocalizationManager::shutdown()
{
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

void LocalizationManager::reloadLangages()
{
	unloadLanguages();

	const std::string locPath = PathUtils::getResourceDirectory() + "\\localization\\*.csv";
	std::vector<std::string> locFiles;
	PathUtils::fetchFilenamesInDirectory(locPath, locFiles);

	for (auto baseFileName : locFiles)
	{
		std::string filename = PathUtils::getResourceDirectory() + "\\localization\\" + baseFileName;
		std::string baseFileNameNoExt = PathUtils::removeFileExtension(baseFileName);
		std::vector<std::string> parts= StringUtils::splitString(baseFileNameNoExt, '_');

		if (parts.size() == 2)
		{
			const std::string& tableName= parts[0];
			const std::string& langCode = parts[1];

			// Fetch the language
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

			// Fetch the string table
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

			io::CSVReader<2> in(filename);
			in.read_header(io::ignore_extra_column, "key", "text");
			
			std::string key; char* utf8Text;
			while (in.read_row(key, utf8Text))
			{
				auto keyIt= stringTable->keyToTextMap.find(key);
				if (keyIt == stringTable->keyToTextMap.end())
				{
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
					std::wstring myunicodestr = convert.from_bytes(utf8Text);

					StringEntry entry= { utf8Text, myunicodestr };

					stringTable->keyToTextMap.insert({ key, entry });
				}
				else
				{
					MIKAN_LOG_WARNING("LocalizationManager::reloadLangages") << 
						"Duplicate key \'" << key << "\' in table \'" << tableName << "\'";
				}
			}
		}
		else
		{
			MIKAN_LOG_WARNING("LocalizationManager::reloadLangages") << "Malformed loc filename: " << filename;
		}
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

const char* locTextUTF8(const char* tableName, const char* stringKey, bool* outHasString)
{
	return LocalizationManager::getInstance()->fetchUTF8Text(tableName, stringKey, outHasString);
}

const wchar_t* locTextUTF16(const char* tableName, const char* stringKey, bool* outHasString)
{
	return LocalizationManager::getInstance()->fetchUTF16Text(tableName, stringKey, outHasString);
}