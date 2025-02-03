#include "NodeEvaluator.h"
#include "Nodes/Node.h"
#include "Pins/FlowPin.h"
#include "Pins/NodeLink.h"

bool NodeEvaluator::evaluateFlowPinChain(NodePtr startNode)
{
	m_currentNode= startNode;
	m_evaluatedNodeCount= 0;

	// Execute node along the FlowPin links until:
	// * We encounter a node with now output FlowPin
	// * Node evaluation returns an error
	// * Node evaluation count hits infinite loop detection threshold
	while (m_currentNode && 
		   !hasErrors() &&
		   m_evaluatedNodeCount < kInifiniteLoopThreshold)
	{
		m_currentNode->evaluateNode(*this);
		m_evaluatedNodeCount++;

		// See if we exceeded the infinite loop threshold
		if (!hasErrors() && m_evaluatedNodeCount >= kInifiniteLoopThreshold)
		{
			addError(NodeEvaluationError(eNodeEvaluationErrorCode::infiniteLoop, "Infinite loop detected"));
		}

		// Try moving on to the next node if there are no errors
		if (!hasErrors())
		{
			FlowPinPtr outputFlowPin= m_currentNode->getOutputFlowPin();

			if (outputFlowPin)
			{
				auto connections= outputFlowPin->getConnectedLinks();
				NodeLinkPtr outputLink= (connections.size() > 0) ? connections[0] : NodeLinkPtr();
				NodePinPtr inputFlowPin= (outputLink) ? outputLink->getEndPin() : NodePinPtr();

				m_currentNode= inputFlowPin ? inputFlowPin->getOwnerNode() : NodePtr();
			}
		}
	}

	return !hasErrors();
}