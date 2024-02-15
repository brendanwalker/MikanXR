#pragma once

#include "NodeError.h"
#include "Nodes/Node.h"
#include "Pins/NodePin.h"

NodeEvaluationError::NodeEvaluationError(
	eNodeEvaluationErrorCode inErrorCode,
	const std::string& inErrorMessage,
	class Node* inErrorNode,
	class NodePin* inErrorPin)
	: errorCode(inErrorCode)
	, errorMessage(inErrorMessage)
	, errorNodeId(inErrorNode ? inErrorNode->getId() : -1)
	, errorPinId(inErrorPin ? inErrorPin->getId() : -1)
{
}
