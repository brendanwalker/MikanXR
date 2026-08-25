#pragma once

#include "TransactionTypes.h"

#include <filesystem>
#include <fstream>
#include <string>

/// Append-only JSONL session log for the transaction history.
/// One file per project-load session under `<projectFolder>/logs/`,
/// flushed per line so the record survives a crash. Lines:
///   {"event":"session","project":...,"started":...}
///   {"event":"txn","seq":N,...,"ops":[...]}
///   {"event":"undo"|"redo","seq":N,...}
///   {"event":"clear",...}
class TransactionLogWriter
{
public:
	static const int k_maxRetainedLogFiles= 20;

	/// Open a new session log next to the project file, pruning old logs.
	bool open(const std::filesystem::path& projectFilePath);
	void close();
	bool isOpen() const { return m_stream.is_open(); }
	const std::filesystem::path& getLogFilePath() const { return m_logFilePath; }

	void writeTransaction(const Transaction& transaction);
	void writeEvent(const char* eventName, int64_t sequenceNumber= -1);

	/// Current time as epoch milliseconds (the timestamp stored on transactions).
	static int64_t nowEpochMs();

private:
	void writeLine(const configuru::Config& lineConfig);
	void pruneOldLogFiles(const std::filesystem::path& logFolder) const;

	std::filesystem::path m_logFilePath;
	std::ofstream m_stream;
};
