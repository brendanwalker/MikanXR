#include "ARKitDebugProtocol.h"

#include <cctype>
#include <cstdlib>

namespace
{
// In LogSeverityLevel order, so the index is the level value. The same
// vocabulary `log tail` accepts, which is what lets a relayed line be filtered
// by the severity the phone chose for it.
const char* k_levelNames[]= {"trace", "debug", "info", "warning", "error", "fatal"};
const int k_levelNameCount= (int)(sizeof(k_levelNames) / sizeof(k_levelNames[0]));

const int k_infoLevel= 2; // LogSeverityLevel::info

// atoi() answers 0 for non-numeric text, which would silently accept "reply x"
// as "reply 0" and "hello v1" as version 0
bool isAllDigits(const std::string& text)
{
	if (text.empty())
		return false;

	for (char character : text)
	{
		if (std::isdigit((unsigned char)character) == 0)
			return false;
	}
	return true;
}
} // namespace

namespace ARKitDebugProtocol
{
void splitFirstWord(const std::string& line, std::string& outWord, std::string& outRest)
{
	const size_t wordEnd= line.find_first_of(" \t");
	if (wordEnd == std::string::npos)
	{
		outWord= line;
		outRest.clear();
		return;
	}

	outWord= line.substr(0, wordEnd);

	const size_t restStart= line.find_first_not_of(" \t", wordEnd);
	outRest= restStart == std::string::npos ? std::string() : line.substr(restStart);
}

bool parseHello(const std::string& line, int& outVersion, std::string& outDeviceName)
{
	std::string keyword;
	std::string body;
	splitFirstWord(line, keyword, body);

	if (keyword != "hello")
		return false;

	std::string versionText;
	std::string deviceName;
	splitFirstWord(body, versionText, deviceName);

	if (!isAllDigits(versionText))
		return false;

	outVersion= atoi(versionText.c_str());
	outDeviceName= deviceName;
	return true;
}

eLineKind classifyLine(const std::string& line, std::string& outBody)
{
	std::string keyword;
	splitFirstWord(line, keyword, outBody);

	if (keyword == "log")
		return eLineKind::log;

	if (keyword == "reply")
		return eLineKind::reply;

	return eLineKind::unknown;
}

void parseLogBody(const std::string& body, int& outLevel, std::string& outText)
{
	std::string levelName;
	std::string text;
	splitFirstWord(body, levelName, text);

	for (int levelIndex= 0; levelIndex < k_levelNameCount; ++levelIndex)
	{
		if (levelName == k_levelNames[levelIndex])
		{
			outLevel= levelIndex;
			outText= text;
			return;
		}
	}

	// Unrecognized level: keep the whole body as the message rather than
	// discarding the first word as a level that was never one
	outLevel= k_infoLevel;
	outText= body;
}

bool parseReplyCount(const std::string& body, int maxLines, int& outCount)
{
	std::string countText;
	std::string trailing;
	splitFirstWord(body, countText, trailing);

	if (!isAllDigits(countText))
		return false;

	const long parsedCount= atol(countText.c_str());
	if (parsedCount > (long)maxLines)
		return false;

	outCount= (int)parsedCount;
	return true;
}
} // namespace ARKitDebugProtocol
