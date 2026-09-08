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
	while (m_currentNode && !hasErrors() && m_evaluatedNodeCount < kInifiniteLoopThreshold)
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
				// Ask the pin for the far end of its link rather than reading the link's stored end
				// pin. A link dragged from an input pin onto an output pin is saved with its ends
				// swapped, and the stored end pin is then this node's own output pin, which walks
				// the chain straight back into the node it just evaluated.
				NodePinPtr inputFlowPin= outputFlowPin->getConnectedTargetPin();

				m_currentNode= inputFlowPin ? inputFlowPin->getOwnerNode() : NodePtr();
			}
		}
	}

	return !hasErrors();
}