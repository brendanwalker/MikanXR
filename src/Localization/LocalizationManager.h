#pragma once

#include <map>
#include <string>
#include <vector>

typedef std::vector<std::string> t_language_tags;

class LocalizationManager
{
public:
	LocalizationManager();
	virtual ~LocalizationManager();

	static LocalizationManager* getInstance() { return m_instance; }

	bool startup();
	void reloadLangages();
	void shutdown();

	const std::string& getDefaultLanguage() const;
	t_language_tags getSystemLanguage() const;
	bool isLanguageSupported(const char *langCode) const;
	std::vector<std::string> getSupportedLanguages() const;
	bool setLanguage(const t_language_tags& langCodes);
	bool setLanguage(const std::string& languageId);
	const std::string& getLanguage() const { return m_currentLanguageCode; }
	const char* fetchUTF8Text(const char* tableName, const char* stringKey);
	const wchar_t* fetchUTF16Text(const char* tableName, const char* stringKey);

private:
	void unloadLanguages();

	static LocalizationManager* m_instance;

	struct StringEntry
	{
		const std::string utf8Text;
		const std::wstring utf16Text;
	};

	struct StringTable
	{
		std::map<std::string, StringEntry> keyToTextMap;
	};

	struct Language
	{
		std::map<std::string, StringTable*> stringTables;
	};

	std::map<std::string, Language*> m_languages;
	std::string m_currentLanguageCode;
	Language* m_currentLanguage;
};

const char* locTextUTF8(const char* tableName, const char* stringKey);
const wchar_t* locTextUTF16(const char* tableName, const char* stringKey);