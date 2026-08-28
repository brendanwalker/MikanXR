#include "JsonlSessionLogWriter.h"
#include "Logger.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>

namespace
{
// Compact single-line JSON for the JSONL format
configuru::FormatOptions makeJsonLineFormat()
{
	configuru::FormatOptions options= configuru::make_json_options();
	options.indentation= "";
	return options;
}
} // namespace

int64_t JsonlSessionLogWriter::nowEpochMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
		.count();
}

std::string JsonlSessionLogWriter::makeLocalTimestampString(const char* format)
{
	const std::time_t now= std::time(nullptr);
	std::tm localTime{};
#if defined(_WIN32)
	localtime_s(&localTime, &now);
#else
	localtime_r(&now, &localTime);
#endif

	char buffer[64];
	std::strftime(buffer, sizeof(buffer), format, &localTime);
	return buffer;
}

bool JsonlSessionLogWriter::openLogFile(const std::filesystem::path& logFolder, const std::string& filePrefix)
{
	close();

	std::error_code errorCode;
	std::filesystem::create_directories(logFolder, errorCode);
	if (errorCode)
	{
		MIKAN_LOG_ERROR("JsonlSessionLogWriter") << "Failed to create log folder " << logFolder.string();
		return false;
	}

	pruneOldLogFiles(logFolder, filePrefix);

	const std::string fileName= filePrefix + "_" + makeLocalTimestampString("%Y%m%d_%H%M%S") + ".jsonl";
	m_logFilePath= logFolder / fileName;

	m_stream.open(m_logFilePath, std::ios::out | std::ios::app);
	if (!m_stream.is_open())
	{
		MIKAN_LOG_ERROR("JsonlSessionLogWriter") << "Failed to open " << m_logFilePath.string();
		return false;
	}

	return true;
}

void JsonlSessionLogWriter::close()
{
	if (m_stream.is_open())
	{
		m_stream.close();
	}
	m_logFilePath.clear();
}

void JsonlSessionLogWriter::writeEvent(const char* eventName, int64_t sequenceNumber)
{
	if (!isOpen())
		return;

	configuru::Config eventLine= configuru::Config::object();
	eventLine["event"]= eventName;
	if (sequenceNumber >= 0)
		eventLine["seq"]= sequenceNumber;
	eventLine["t"]= nowEpochMs();
	writeLine(eventLine);
}

void JsonlSessionLogWriter::writeLine(const configuru::Config& lineConfig)
{
	static const configuru::FormatOptions k_lineFormat= makeJsonLineFormat();

	m_stream << configuru::dump_string(lineConfig, k_lineFormat) << "\n";
	m_stream.flush();
}

void JsonlSessionLogWriter::pruneOldLogFiles(const std::filesystem::path& logFolder,
											 const std::string& filePrefix) const
{
	const std::string namePrefix= filePrefix + "_";
	std::vector<std::filesystem::path> logFiles;

	std::error_code errorCode;
	for (const auto& entry : std::filesystem::directory_iterator(logFolder, errorCode))
	{
		const std::string fileName= entry.path().filename().string();
		if (entry.is_regular_file() && fileName.rfind(namePrefix, 0) == 0 && entry.path().extension() == ".jsonl")
		{
			logFiles.push_back(entry.path());
		}
	}

	if ((int)logFiles.size() < k_maxRetainedLogFiles)
		return;

	// Session-stamped names sort chronologically; drop the oldest so the
	// folder holds at most k_maxRetainedLogFiles including the new session
	std::sort(logFiles.begin(), logFiles.end());
	const int removeCount= (int)logFiles.size() - (k_maxRetainedLogFiles - 1);
	for (int i= 0; i < removeCount; ++i)
	{
		std::filesystem::remove(logFiles[i], errorCode);
	}
}
