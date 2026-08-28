#pragma once

#include "JsonlSessionLogWriter.h"
#include "TransactionTypes.h"

#include <filesystem>

/// Append-only JSONL session log for the transaction history.
/// One file per project-load session under `<projectFolder>/logs/`,
/// flushed per line so the record survives a crash. Lines:
///   {"event":"session","project":...,"started":...}
///   {"event":"txn","seq":N,...,"ops":[...]}
///   {"event":"undo"|"redo","seq":N,...}
///   {"event":"clear",...}
class TransactionLogWriter : public JsonlSessionLogWriter
{
public:
	/// Open a new session log next to the project file, pruning old logs.
	bool open(const std::filesystem::path& projectFilePath);

	void writeTransaction(const Transaction& transaction);
};
