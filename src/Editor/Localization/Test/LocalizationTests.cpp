#include "LocalizationTests.h"
#include "LocalizationManager.h"
#include "MkGuiTheme.h"
#include "PathUtils.h"

#include "unit_test.h"

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace
{
// Loads the bundled tables only (no CDN cache overlay, no background fetch) so
// the result depends on the repo, not on what this machine has downloaded
static bool startupTestManager(LocalizationManager& manager)
{
	const std::filesystem::path localizationDir= PathUtils::getResourceDirectory() / "localization";

	return manager.startup(localizationDir, AppSettingsConfigPtr(), /*bEnableCommunityTranslations=*/false);
}

static bool decodeNextCodepoint(const std::string& text, size_t& inOutIndex, unsigned int& outCodepoint)
{
	const unsigned char lead= (unsigned char)text[inOutIndex];
	size_t extraBytes= 0;
	if (lead < 0x80)
	{
		outCodepoint= lead;
	}
	else if ((lead & 0xE0) == 0xC0)
	{
		outCodepoint= lead & 0x1F;
		extraBytes= 1;
	}
	else if ((lead & 0xF0) == 0xE0)
	{
		outCodepoint= lead & 0x0F;
		extraBytes= 2;
	}
	else if ((lead & 0xF8) == 0xF0)
	{
		outCodepoint= lead & 0x07;
		extraBytes= 3;
	}
	else
	{
		return false;
	}

	for (size_t byteIndex= 0; byteIndex < extraBytes; ++byteIndex)
	{
		++inOutIndex;
		if (inOutIndex >= text.size() || ((unsigned char)text[inOutIndex] & 0xC0) != 0x80)
			return false;
		outCodepoint= (outCodepoint << 6) | ((unsigned char)text[inOutIndex] & 0x3F);
	}
	++inOutIndex;
	return true;
}

static bool isCodepointInRanges(unsigned int codepoint, const ImWchar* ranges)
{
	for (const ImWchar* range= ranges; range[0] != 0; range+= 2)
	{
		if (codepoint >= range[0] && codepoint <= range[1])
			return true;
	}
	return false;
}
} // namespace

bool localization_test_tables_load_clean()
{
	UNIT_TEST_BEGIN("tables load without validation warnings")

	LocalizationManager manager;
	if (!startupTestManager(manager))
	{
		fprintf(stdout, "    FAILED: manager startup (missing or corrupt en.json)\n");
		manager.shutdown();
		success= false;
		UNIT_TEST_COMPLETE()
	}

	for (const std::string& warning : manager.getLoadWarnings())
	{
		fprintf(stdout, "    load warning: %s\n", warning.c_str());
		success= false;
	}

	manager.shutdown();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool localization_test_window_titles_are_unique_ids()
{
	UNIT_TEST_BEGIN("window titles are non-empty and unique")

	LocalizationManager manager;
	if (!startupTestManager(manager))
	{
		fprintf(stdout, "    FAILED: manager startup\n");
		manager.shutdown();
		success= false;
		UNIT_TEST_COMPLETE()
	}

	const std::map<std::string, std::string>* englishStrings= manager.getRawStrings("en");
	if (englishStrings == nullptr)
	{
		fprintf(stdout, "    FAILED: no English table loaded\n");
		manager.shutdown();
		success= false;
		UNIT_TEST_COMPLETE()
	}

	// The English text of every windows.* key IS the window's ImGui ID, so an
	// empty or duplicated title would merge two windows
	std::map<std::string, std::string> titleToKey;
	for (const auto& [key, text] : *englishStrings)
	{
		if (key.rfind("windows.", 0) != 0)
			continue;

		if (text.empty())
		{
			fprintf(stdout, "    FAILED: empty window title: %s\n", key.c_str());
			success= false;
			continue;
		}

		const auto [existingIt, bInserted]= titleToKey.emplace(text, key);
		if (!bInserted)
		{
			fprintf(stdout, "    FAILED: duplicate window title '%s' (%s vs %s)\n", text.c_str(), key.c_str(),
					existingIt->second.c_str());
			success= false;
		}
	}

	manager.shutdown();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool localization_test_unknown_key_passes_through()
{
	UNIT_TEST_BEGIN("unknown key returns the key itself")

	LocalizationManager manager;
	if (!startupTestManager(manager))
	{
		fprintf(stdout, "    FAILED: manager startup\n");
		manager.shutdown();
		success= false;
		UNIT_TEST_COMPLETE()
	}

	const char* bogusKey= "bogus.key";
	success&= (manager.fetchText(bogusKey) == bogusKey);
	success&= (manager.fetchLabel(bogusKey) == bogusKey);
	success&= (manager.fetchWindowTitle(bogusKey) == bogusKey);

	manager.shutdown();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool localization_test_glyph_coverage()
{
	UNIT_TEST_BEGIN("every string renders with the baked glyph ranges")

	LocalizationManager manager;
	if (!startupTestManager(manager))
	{
		fprintf(stdout, "    FAILED: manager startup\n");
		manager.shutdown();
		success= false;
		UNIT_TEST_COMPLETE()
	}

	const ImWchar* glyphRanges= MkGuiTheme::getUiGlyphRanges();
	size_t checkedStringCount= 0;

	for (const LocalizationManager::LanguageInfo& info : manager.getSupportedLanguageInfos())
	{
		const std::map<std::string, std::string>* rawStrings= manager.getRawStrings(info.code);
		if (rawStrings == nullptr)
			continue;

		// The native language name is displayed in the language selector, so
		// it has to render too
		std::vector<std::pair<std::string, std::string>> checkStrings(rawStrings->begin(), rawStrings->end());
		checkStrings.emplace_back("_meta.nativeName", info.nativeName);

		for (const auto& [key, text] : checkStrings)
		{
			++checkedStringCount;
			size_t byteIndex= 0;
			while (byteIndex < text.size())
			{
				unsigned int codepoint= 0;
				if (!decodeNextCodepoint(text, byteIndex, codepoint))
				{
					fprintf(stdout, "    FAILED: %s: '%s' has malformed UTF-8\n", info.code.c_str(), key.c_str());
					success= false;
					break;
				}

				if (codepoint < 0x20)
					continue; // control characters (\n) are not glyphs

				if (!isCodepointInRanges(codepoint, glyphRanges))
				{
					fprintf(stdout, "    FAILED: %s: '%s' uses codepoint U+%04X outside the baked glyph ranges\n",
							info.code.c_str(), key.c_str(), codepoint);
					success= false;
				}
			}
		}
	}

	fprintf(stdout, "      %zu string(s) glyph-checked\n", checkedStringCount);

	manager.shutdown();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool run_localization_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("localization")
	UNIT_TEST_MODULE_CALL_TEST(localization_test_tables_load_clean);
	UNIT_TEST_MODULE_CALL_TEST(localization_test_window_titles_are_unique_ids);
	UNIT_TEST_MODULE_CALL_TEST(localization_test_unknown_key_passes_through);
	UNIT_TEST_MODULE_CALL_TEST(localization_test_glyph_coverage);
	UNIT_TEST_MODULE_END()
}
