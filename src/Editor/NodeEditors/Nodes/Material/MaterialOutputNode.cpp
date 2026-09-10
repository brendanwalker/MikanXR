#include "MaterialOutputNode.h"
#include "IconsForkAwesome.h"
#include "NodeEditorState.h"
#include "Graphs/MaterialNodeGraph.h"
#include "Graphs/NodeGraph.h"

void MaterialOutputNode::rebuildPinsForDomain(eMaterialDomain domain)
{
	// Keep the color pin and its link across a domain change; only the
	// domain-specific pins come and go
	ShaderValuePinPtr positionOffsetPin= getPositionOffsetPin();
	const bool bWantsPositionOffset= (domain == eMaterialDomain::shape);

	if (positionOffsetPin && !bWantsPositionOffset)
	{
		// The graph deletes the pin's links and detaches it from this node
		if (m_ownerGraph)
		{
			m_ownerGraph->deletePinById(positionOffsetPin->getId());
		}
	}
	else if (!positionOffsetPin && bWantsPositionOffset)
	{
		addShaderInputPin(k_positionOffsetPinName, eShaderValueType::float3, {0.f, 0.f, 0.f, 0.f});
	}

	if (!getColorPin())
	{
		addShaderInputPin(k_colorPinName, eShaderValueType::float4, {0.f, 0.f, 0.f, 1.f});
	}
}

const char* MaterialOutputNode::editorGetHeaderIcon() const { return ICON_FK_PAINT_BRUSH; }

ImVec4 MaterialOutputNode::editorGetHeaderColor() const
{
	return ImVec4(150.f / 255.f, 96.f / 255.f, 110.f / 255.f, 225.f / 255.f);
}

// -- MaterialOutputNodeFactory -----
NodePtr MaterialOutputNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<MaterialOutputNode>(NodeFactory::createNode(editorState));

	auto materialGraph= std::dynamic_pointer_cast<MaterialNodeGraph>(editorState.nodeGraph);
	const eMaterialDomain domain= materialGraph ? materialGraph->getDomain() : eMaterialDomain::compositor;

	node->addShaderInputPin(MaterialOutputNode::k_colorPinName, eShaderValueType::float4, {0.f, 0.f, 0.f, 1.f});
	if (domain == eMaterialDomain::shape)
	{
		node->addShaderInputPin(MaterialOutputNode::k_positionOffsetPinName, eShaderValueType::float3,
								{0.f, 0.f, 0.f, 0.f});
	}

	return node;
}
