#include "NodeLinkDirectionTests.h"
#include "unit_test.h"

#include "Pins/NodeLink.h"
#include "Pins/NodePin.h"

namespace
{
NodePinPtr makePin(t_node_pin_id id, eNodePinDirection direction)
{
	NodePinPtr pin= std::make_shared<NodePin>();
	pin->setId(id);
	pin->setDirection(direction);

	return pin;
}

// Wires a link with its ends in the given order and attaches it to both pins, which is what
// NodeGraph::createLink and the graph loader both end up doing.
NodeLinkPtr makeLink(t_node_link_id id, NodePinPtr startPin, NodePinPtr endPin)
{
	NodeLinkPtr link= std::make_shared<NodeLink>();
	link->setId(id);
	link->setStartPin(startPin);
	link->setEndPin(endPin);

	startPin->connectLink(link);
	endPin->connectLink(link);

	return link;
}
} // namespace

// -- tests -----

bool node_link_direction_test_forward_link_resolves_both_ends()
{
	UNIT_TEST_BEGIN("a link stored output to input resolves from either end")

	NodePinPtr outputPin= makePin(1, eNodePinDirection::OUTPUT);
	NodePinPtr inputPin= makePin(2, eNodePinDirection::INPUT);
	makeLink(10, outputPin, inputPin);

	success&= outputPin->getConnectedTargetPin() == inputPin;
	success&= inputPin->getConnectedSourcePin() == outputPin;

	UNIT_TEST_COMPLETE()
}

bool node_link_direction_test_reversed_link_resolves_both_ends()
{
	UNIT_TEST_BEGIN("a link stored input to output resolves the same way")

	// Dragging from an input pin onto an output pin is allowed, and stores the link with its ends
	// swapped. Reading the ends positionally then hands a pin back its own self, which is what
	// stalled the compositor flow chain at the node feeding a DrawShapesNode.
	NodePinPtr outputPin= makePin(1, eNodePinDirection::OUTPUT);
	NodePinPtr inputPin= makePin(2, eNodePinDirection::INPUT);
	makeLink(10, inputPin, outputPin);

	success&= outputPin->getConnectedTargetPin() == inputPin;
	success&= inputPin->getConnectedSourcePin() == outputPin;

	UNIT_TEST_COMPLETE()
}

bool node_link_direction_test_target_pin_is_output_only()
{
	UNIT_TEST_BEGIN("target and source lookups only answer for their own pin direction")

	NodePinPtr outputPin= makePin(1, eNodePinDirection::OUTPUT);
	NodePinPtr inputPin= makePin(2, eNodePinDirection::INPUT);
	makeLink(10, outputPin, inputPin);

	// Each accessor names the direction it walks, so the opposite direction answers nothing rather
	// than silently walking the link backwards
	success&= inputPin->getConnectedTargetPin() == nullptr;
	success&= outputPin->getConnectedSourcePin() == nullptr;

	// An unconnected pin has nothing to resolve either
	NodePinPtr lonePin= makePin(3, eNodePinDirection::OUTPUT);
	success&= lonePin->getConnectedTargetPin() == nullptr;

	UNIT_TEST_COMPLETE()
}

// -- test module -----

bool run_node_link_direction_tests()
{
	UNIT_TEST_MODULE_BEGIN("node_link_direction")
	UNIT_TEST_MODULE_CALL_TEST(node_link_direction_test_forward_link_resolves_both_ends);
	UNIT_TEST_MODULE_CALL_TEST(node_link_direction_test_reversed_link_resolves_both_ends);
	UNIT_TEST_MODULE_CALL_TEST(node_link_direction_test_target_pin_is_output_only);
	UNIT_TEST_MODULE_END()
}
