#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // This function or variable may be unsafe
#pragma warning(disable : 4244) // conversion from 'const int64_t' to 'float', possible loss of data
#pragma warning(disable : 4715) // configuru::Config::operator[]': not all control paths return a value
#endif
#include <configuru.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/// Append-only JSONL session log base: a timestamped `<prefix>_*.jsonl` file
/// under a log folder, pruned per prefix, one compact JSON object per line,
/// flushed per line so the record survives a crash. Derived writers open the
/// file and define their own line schemas.
class JsonlSessionLogWriter
{
public:
	static const int k_maxRetainedLogFiles= 20;

	bool isOpen() const { return m_stream.is_open(); }
	const std::filesystem::path& getLogFilePath() const { return m_logFilePath; }
	void close();

	void writeEvent(const char* eventName, int64_t sequenceNumber= -1);

	/// Current time as epoch milliseconds (the timestamp stored on log lines).
	static int64_t nowEpochMs();

	/// Local time formatted with strftime.
	static std::string makeLocalTimestampString(const char* format);

protected:
	/// Create the folder, prune old `<filePrefix>_*.jsonl` files, and open a
	/// new `<filePrefix>_<timestamp>.jsonl`. The derived writer appends its
	/// own session line.
	bool openLogFile(const std::filesystem::path& logFolder, const std::string& filePrefix);

	void writeLine(const configuru::Config& lineConfig);

private:
	void pruneOldLogFiles(const std::filesystem::path& logFolder, const std::string& filePrefix) const;

	std::filesystem::path m_logFilePath;
	std::ofstream m_stream;
};
