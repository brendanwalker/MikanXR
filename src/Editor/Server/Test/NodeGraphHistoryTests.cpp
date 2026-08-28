#include "NodeGraphHistoryTests.h"
#include "unit_test.h"

#include "Graphs/NodeGraphHistory.h"

#include <cstdio>
#include <string>

// -- tests -----

bool node_graph_history_test_commit_and_dedup()
{
	UNIT_TEST_BEGIN("commit pushes new snapshots and dedups identical ones")

	NodeGraphHistory history;
	history.reset("A");
	success&= history.getDepth() == 1;
	success&= history.getCursor() == 0;
	success&= !history.canUndo();
	success&= !history.canRedo();

	// An identical snapshot is a no-op
	success&= !history.commit("A");
	success&= history.getDepth() == 1;

	// A different snapshot pushes and advances the cursor
	success&= history.commit("B");
	success&= history.getDepth() == 2;
	success&= history.getCursor() == 1;
	success&= history.canUndo();
	success&= !history.canRedo();

	UNIT_TEST_COMPLETE()
}

bool node_graph_history_test_undo_redo_stepping()
{
	UNIT_TEST_BEGIN("undo and redo step the cursor and return the target snapshot")

	NodeGraphHistory history;
	history.reset("A");
	history.commit("B");
	history.commit("C");

	const std::string* snapshot= history.undo();
	success&= snapshot != nullptr && *snapshot == "B";
	snapshot= history.undo();
	success&= snapshot != nullptr && *snapshot == "A";
	success&= history.undo() == nullptr;
	success&= history.canRedo();

	snapshot= history.redo();
	success&= snapshot != nullptr && *snapshot == "B";
	snapshot= history.redo();
	success&= snapshot != nullptr && *snapshot == "C";
	success&= history.redo() == nullptr;

	UNIT_TEST_COMPLETE()
}

bool node_graph_history_test_commit_truncates_redo_tail()
{
	UNIT_TEST_BEGIN("committing after an undo truncates the redo tail")

	NodeGraphHistory history;
	history.reset("A");
	history.commit("B");
	history.commit("C");
	history.undo();
	history.undo();

	success&= history.commit("D");
	success&= history.getDepth() == 2;
	success&= history.getCursor() == 1;
	success&= !history.canRedo();

	const std::string* snapshot= history.undo();
	success&= snapshot != nullptr && *snapshot == "A";

	UNIT_TEST_COMPLETE()
}

bool node_graph_history_test_depth_cap_drops_oldest()
{
	UNIT_TEST_BEGIN("the depth cap drops the oldest snapshot")

	NodeGraphHistory history;
	history.reset("base");
	for (size_t i= 0; i < NodeGraphHistory::k_maxHistoryDepth + 10; i++)
	{
		history.commit("snapshot" + std::to_string(i));
	}

	success&= history.getDepth() == NodeGraphHistory::k_maxHistoryDepth;
	success&= history.getCursor() == history.getDepth() - 1;

	// Undo all the way back lands on the oldest retained snapshot, not "base"
	const std::string* snapshot= nullptr;
	const std::string* lastSnapshot= nullptr;
	while ((snapshot= history.undo()) != nullptr)
	{
		lastSnapshot= snapshot;
	}
	success&= lastSnapshot != nullptr && *lastSnapshot != "base";

	UNIT_TEST_COMPLETE()
}

bool node_graph_history_test_reset_clears_history()
{
	UNIT_TEST_BEGIN("reset replaces the history with a new baseline")

	NodeGraphHistory history;
	history.reset("A");
	history.commit("B");
	history.reset("C");

	success&= history.getDepth() == 1;
	success&= history.getCursor() == 0;
	success&= !history.canUndo();
	success&= !history.canRedo();

	UNIT_TEST_COMPLETE()
}

// -- test module -----

bool run_node_graph_history_tests()
{
	UNIT_TEST_MODULE_BEGIN("node_graph_history")
	UNIT_TEST_MODULE_CALL_TEST(node_graph_history_test_commit_and_dedup);
	UNIT_TEST_MODULE_CALL_TEST(node_graph_history_test_undo_redo_stepping);
	UNIT_TEST_MODULE_CALL_TEST(node_graph_history_test_commit_truncates_redo_tail);
	UNIT_TEST_MODULE_CALL_TEST(node_graph_history_test_depth_cap_drops_oldest);
	UNIT_TEST_MODULE_CALL_TEST(node_graph_history_test_reset_clears_history);
	UNIT_TEST_MODULE_END()
}
