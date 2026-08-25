#include "TransactionLogWriter.h"
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

std::string makeLocalTimestampString(const char* format)
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

const char* opKindToString(eTransactionOpKind kind)
{
	switch (kind)
	{
	case eTransactionOpKind::setProperty:
		return "set";
	case eTransactionOpKind::createObject:
		return "create";
	case eTransactionOpKind::destroyObject:
		return "destroy";
	default:
		return "unknown";
	}
}
} // namespace

int64_t TransactionLogWriter::nowEpochMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
		.count();
}

bool TransactionLogWriter::open(const std::filesystem::path& projectFilePath)
{
	close();

	const std::filesystem::path logFolder= projectFilePath.parent_path() / "logs";

	std::error_code errorCode;
	std::filesystem::create_directories(logFolder, errorCode);
	if (errorCode)
	{
		MIKAN_LOG_ERROR("TransactionLogWriter") << "Failed to create log folder " << logFolder.string();
		return false;
	}

	pruneOldLogFiles(logFolder);

	const std::string fileName= "transactions_" + makeLocalTimestampString("%Y%m%d_%H%M%S") + ".jsonl";
	m_logFilePath= logFolder / fileName;

	m_stream.open(m_logFilePath, std::ios::out | std::ios::app);
	if (!m_stream.is_open())
	{
		MIKAN_LOG_ERROR("TransactionLogWriter") << "Failed to open " << m_logFilePath.string();
		return false;
	}

	configuru::Config sessionLine= configuru::Config::object();
	sessionLine["event"]= "session";
	sessionLine["project"]= projectFilePath.string();
	sessionLine["started"]= makeLocalTimestampString("%Y-%m-%dT%H:%M:%S");
	writeLine(sessionLine);

	MIKAN_LOG_INFO("TransactionLogWriter") << "Transaction log: " << m_logFilePath.string();
	return true;
}

void TransactionLogWriter::close()
{
	if (m_stream.is_open())
	{
		m_stream.close();
	}
	m_logFilePath.clear();
}

void TransactionLogWriter::writeTransaction(const Transaction& transaction)
{
	if (!isOpen())
		return;

	configuru::Config opsArray= configuru::Config::array();
	for (const TransactionOp& op : transaction.ops)
	{
		configuru::Config opConfig= configuru::Config::object();
		opConfig["op"]= opKindToString(op.kind);
		opConfig["system"]= op.systemName;
		if (!op.componentClassName.empty())
			opConfig["class"]= op.componentClassName;
		opConfig["id"]= op.componentId;

		if (op.kind == eTransactionOpKind::setProperty)
		{
			opConfig["prop"]= op.propertyName;
			opConfig["type"]= mikanVariantTypeToString(op.valueType);
			opConfig["old"]= op.oldValueText;
			opConfig["new"]= op.newValueText;
		}
		else
		{
			opConfig["def"]= op.definitionConfig;
		}

		opsArray.push_back(opConfig);
	}

	configuru::Config txnLine= configuru::Config::object();
	txnLine["event"]= "txn";
	txnLine["seq"]= transaction.sequenceNumber;
	txnLine["t"]= transaction.timestampMs;
	txnLine["desc"]= transaction.description;
	if (!transaction.gestureId.empty())
		txnLine["gesture"]= transaction.gestureId;
	txnLine["ops"]= opsArray;
	writeLine(txnLine);
}

void TransactionLogWriter::writeEvent(const char* eventName, int64_t sequenceNumber)
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

void TransactionLogWriter::writeLine(const configuru::Config& lineConfig)
{
	static const configuru::FormatOptions k_lineFormat= makeJsonLineFormat();

	m_stream << configuru::dump_string(lineConfig, k_lineFormat) << "\n";
	m_stream.flush();
}

void TransactionLogWriter::pruneOldLogFiles(const std::filesystem::path& logFolder) const
{
	std::vector<std::filesystem::path> logFiles;

	std::error_code errorCode;
	for (const auto& entry : std::filesystem::directory_iterator(logFolder, errorCode))
	{
		const std::string fileName= entry.path().filename().string();
		if (entry.is_regular_file() && fileName.rfind("transactions_", 0) == 0 && entry.path().extension() == ".jsonl")
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
