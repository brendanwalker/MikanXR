#pragma once

#include <string>
#include <vector>

/// Per-editor-window undo history for a node graph: a bounded stack of
/// whole-graph JSON snapshots with a cursor. Node graphs have no per-property
/// capture chokepoint, so undo restores a snapshot through the graph's config
/// load path rather than replaying ops.
class NodeGraphHistory
{
public:
	static const size_t k_maxHistoryDepth= 64;

	/// Clear the history and seed entry 0 with the graph's current state.
	void reset(const std::string& baselineSnapshot);
	void clear();

	/// Push a new snapshot if it differs from the snapshot at the cursor,
	/// truncating any redo tail. Drops the oldest entry at the depth cap.
	/// @returns true if a new entry was pushed
	bool commit(const std::string& snapshot);

	bool canUndo() const { return m_cursor > 0; }
	bool canRedo() const { return m_cursor + 1 < m_snapshots.size(); }

	/// Step the cursor and return the snapshot to restore,
	/// or nullptr when at the corresponding end of the stack.
	const std::string* undo();
	const std::string* redo();

	size_t getDepth() const { return m_snapshots.size(); }
	size_t getCursor() const { return m_cursor; }

private:
	// m_snapshots[m_cursor] is the applied state; entries past it are the redo tail
	std::vector<std::string> m_snapshots;
	size_t m_cursor= 0;
};
