#pragma once

#include "JsonlSessionLogWriter.h"

#include <array>
#include <filesystem>
#include <string>

/// Append-only JSONL session log for a node editor window, the node graph
/// counterpart to the transaction history log. One file per window session
/// under `<projectFolder>/logs/`, each committed undo step carrying the whole
/// graph snapshot so a session can be reconstructed after the fact. Lines:
///   {"event":"session","project":...,"window":...,"started":...}
///   {"event":"baseline","graph":...,"class":...,"snapshot":{...}}
///   {"event":"txn","seq":N,"desc":...,"snapshot":{...}}
///   {"event":"undo"|"redo","steps":N,"cursor":C,...}
///   {"event":"restore_failed",...}
class NodeGraphLogWriter : public JsonlSessionLogWriter
{
public:
	/// Open a new session log next to the project file, pruning old logs.
	bool open(const std::filesystem::path& projectFilePath, const std::string& windowClassName);

	/// The graph state a session (or a graph load within it) starts from.
	void writeBaseline(const std::filesystem::path& graphPath, const std::string& graphClassName,
					   const configuru::Config& snapshot);

	/// One committed undo step, carrying the whole-graph snapshot.
	void writeSnapshotCommit(int64_t sequenceNumber, const configuru::Config& snapshot);

	/// An applied undo/redo: the step count, the resulting history cursor,
	/// and the restored snapshot (count tracking only, not re-logged).
	void writeHistoryStep(const char* eventName, int steps, size_t cursor, const configuru::Config& restoredSnapshot);

private:
	/// Collection-count deltas vs the previous snapshot ("nodes 13->14"),
	/// empty when no counts changed. Updates the tracked counts.
	std::string makeCountDeltaDesc(const configuru::Config& snapshot);

	static const int k_collectionCount= 5;
	std::array<int64_t, k_collectionCount> m_lastCounts{};
};
