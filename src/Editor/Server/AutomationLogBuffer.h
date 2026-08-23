#pragma once

#include <string>
#include <vector>

/// Ring buffer of recent log lines, fed by the logger callback installed in
/// App::startup and read back through the automation server's log command.
/// Static because the logger callback is a plain function pointer.
/// Thread safe: log lines arrive from any thread.
class AutomationLogBuffer
{
public:
	static const int k_maxBufferedLines= 2000;

	/// Matches the logger's t_logCallback signature.
	/// level is a LogSeverityLevel value.
	static void logCallback(int level, const char* line);

	/// Copy up to lineCount of the most recent lines at or above minLevel,
	/// oldest first.
	static void getTail(int lineCount, int minLevel, std::vector<std::string>& outLines);
};
