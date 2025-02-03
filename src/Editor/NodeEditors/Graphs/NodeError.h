#pragma once

#include "NodeFwd.h"

#include <string>
#include <vector>

enum eNodeEvaluationErrorCode
{
	NONE = -1,

	invalidNode,
	missingInput,
	missingOutput,
	materialError,
	evaluationError,
	infiniteLoop
};

struct NodeEvaluationError
{
	NodeEvaluationError() = default;
	NodeEvaluationError(
		eNodeEvaluationErrorCode inErrorCode,
		const std::string& inErrorMessage,
		class Node* inErrorNode= nullptr,
		class NodePin* inErrorPin= nullptr);

	eNodeEvaluationErrorCode errorCode = NONE;
	std::string errorMessage;
	t_node_id errorNodeId = -1;
	t_node_pin_id errorPinId = -1;
};