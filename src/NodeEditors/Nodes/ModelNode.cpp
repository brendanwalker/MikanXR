#include "ModelNode.h"
#include "GlMaterial.h"
#include "GlTexture.h"
#include "ModelAssetReference.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/ModelPin.h"
#include "Properties/GraphVariableList.h"
#include "Properties/GraphModelProperty.h"

#include "imgui.h"
#include "imnodes.h"

ModelNode::ModelNode()
	: Node()
{}

ModelNode::ModelNode(NodeGraphPtr ownerGraph)
	: Node(ownerGraph)
{
	if (ownerGraph)
	{
		m_modelArrayProperty = ownerGraph->getTypedPropertyByName<GraphVariableList>("models");
		ownerGraph->OnPropertyModifed += MakeDelegate(this, &ModelNode::onGraphPropertyChanged);
	}
}

ModelNode::~ModelNode()
{
	m_modelArrayProperty = nullptr;
	if (m_ownerGraph)
	{
		m_ownerGraph->OnPropertyModifed -= MakeDelegate(this, &ModelNode::onGraphPropertyChanged);
	}
}

GlRenderModelResourcePtr ModelNode::getModelResource() const
{
	return m_sourceProperty ? m_sourceProperty->getModelResource() : GlRenderModelResourcePtr();
}

void ModelNode::setModelSource(GraphModelPropertyPtr inModelProperty)
{
	m_sourceProperty = inModelProperty;

	ModelPinPtr outPin = getFirstPinOfType<ModelPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(getModelResource());
	}
}

bool ModelNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Only update output pin in setMaterialSource

	return true;
}

void ModelNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

void ModelNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	GlTexturePtr textureResource = m_sourceProperty->getModelAssetReference()->getPreviewTexture();
	uint32_t glTextureId = textureResource ? textureResource->getGlTextureId() : 0;
	ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
	ImGui::SameLine();

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

void ModelNode::onGraphPropertyChanged(t_graph_property_id id)
{
	if (m_modelArrayProperty && m_modelArrayProperty->getId() == id)
	{
		auto modelArray = m_modelArrayProperty->getArray();
		auto it = std::find(modelArray.begin(), modelArray.end(), m_sourceProperty);

		if (it == modelArray.end())
		{
			setModelSource(GraphModelPropertyPtr());
		}
	}
}

// -- ModelNode Factory -----
NodePtr ModelNodeFactory::createNode(const NodeEditorState* editorState) const
{
	// Create the node and pins
	ModelNodePtr node = std::make_shared<ModelNode>();
	ModelPinPtr outputPin = node->addPin<ModelPin>("model", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}