#include "NodeGraphHistory.h"

void NodeGraphHistory::reset(const std::string& baselineSnapshot)
{
	m_snapshots.clear();
	m_snapshots.push_back(baselineSnapshot);
	m_cursor= 0;
}

void NodeGraphHistory::clear()
{
	m_snapshots.clear();
	m_cursor= 0;
}

bool NodeGraphHistory::commit(const std::string& snapshot)
{
	if (m_snapshots.empty())
	{
		reset(snapshot);
		return true;
	}

	if (snapshot == m_snapshots[m_cursor])
	{
		return false;
	}

	// Truncate the redo tail
	m_snapshots.resize(m_cursor + 1);

	// Drop the oldest entry at the depth cap
	if (m_snapshots.size() >= k_maxHistoryDepth)
	{
		m_snapshots.erase(m_snapshots.begin());
	}

	m_snapshots.push_back(snapshot);
	m_cursor= m_snapshots.size() - 1;

	return true;
}

const std::string* NodeGraphHistory::undo()
{
	if (!canUndo())
	{
		return nullptr;
	}

	m_cursor--;
	return &m_snapshots[m_cursor];
}

const std::string* NodeGraphHistory::redo()
{
	if (!canRedo())
	{
		return nullptr;
	}

	m_cursor++;
	return &m_snapshots[m_cursor];
}
