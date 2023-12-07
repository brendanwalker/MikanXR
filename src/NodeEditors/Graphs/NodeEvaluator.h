#pragma once

#include "NodeFwd.h"

#include <string>

enum eNodeEvaluationErrorCode
{
	NONE= -1,

	invalidNode,
	evaluationError,
	infiniteLoop
};

class NodeEvaluator
{
public:
	NodeEvaluator()= default;

	inline void setLastErrorCode(eNodeEvaluationErrorCode errorCode) { m_errorCode= errorCode; }
	inline eNodeEvaluationErrorCode getLastErrorCode() const { return m_errorCode; }

	inline void setLastErrorMessage(const std::string& message) { m_errorMessage= message;  }
	inline const std::string& getLastErrorMessage() const { return m_errorMessage; }

	bool evaluateFlowPinChain(NodePtr startNode);

protected:
	NodePtr m_currentNode;
	eNodeEvaluationErrorCode m_errorCode;
	std::string m_errorMessage;
	int m_evaluatedNodeCount;

	static const int kInifiniteLoopThreshold= 1000;
};
