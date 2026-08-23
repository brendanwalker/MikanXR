#include "AutomationLogBuffer.h"

#include <algorithm>
#include <deque>
#include <mutex>

namespace
{
struct BufferedLogLine
{
	int level;
	std::string text;
};

std::mutex g_logBufferMutex;
std::deque<BufferedLogLine> g_logBuffer;
} // namespace

void AutomationLogBuffer::logCallback(int level, const char* line)
{
	std::lock_guard<std::mutex> lock(g_logBufferMutex);

	g_logBuffer.push_back({level, line != nullptr ? line : ""});
	while (g_logBuffer.size() > (size_t)k_maxBufferedLines)
	{
		g_logBuffer.pop_front();
	}
}

void AutomationLogBuffer::getTail(int lineCount, int minLevel, std::vector<std::string>& outLines)
{
	std::lock_guard<std::mutex> lock(g_logBufferMutex);

	// Walk backward collecting matches, then reverse to oldest-first order
	for (auto it= g_logBuffer.rbegin(); it != g_logBuffer.rend() && (int)outLines.size() < lineCount; ++it)
	{
		if (it->level >= minLevel)
		{
			outLines.push_back(it->text);
		}
	}

	std::reverse(outLines.begin(), outLines.end());
}
