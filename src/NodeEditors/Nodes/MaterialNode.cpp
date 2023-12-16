#include "MaterialNode.h"
#include "GlMaterial.h"
#include "GlTexture.h"
#include "MaterialAssetReference.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/MaterialPin.h"
#include "Properties/GraphVariableList.h"
#include "Properties/GraphMaterialProperty.h"

#include "imgui.h"
#include "imnodes.h"

MaterialNode::MaterialNode()
	: Node()
{}

MaterialNode::MaterialNode(NodeGraphPtr ownerGraph)
	: Node(ownerGraph)
{
	if (ownerGraph)
	{
		m_materialArrayProperty = ownerGraph->getTypedPropertyByName<GraphVariableList>("materials");
		ownerGraph->OnPropertyModifed += MakeDelegate(this, &MaterialNode::onGraphPropertyChanged);
	}
}

MaterialNode::~MaterialNode()
{
	m_materialArrayProperty = nullptr;
	if (m_ownerGraph)
	{
		m_ownerGraph->OnPropertyModifed -= MakeDelegate(this, &MaterialNode::onGraphPropertyChanged);
	}
}

GlMaterialPtr MaterialNode::getMaterialResource() const
{
	return m_sourceProperty ? m_sourceProperty->getMaterialResource() : GlMaterialPtr();
}

void MaterialNode::setMaterialSource(GraphMaterialPropertyPtr inMaterialProperty) 
{ 
	m_sourceProperty = inMaterialProperty; 

	MaterialPinPtr outPin = getFirstPinOfType<MaterialPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(getMaterialResource());
	}
}

bool MaterialNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Only update output pin in setMaterialSource

	return true;
}

void MaterialNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

void MaterialNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	GlTexturePtr textureResource = m_sourceProperty->getMaterialAssetReference()->getPreviewTexture();
	uint32_t glTextureId = textureResource ? textureResource->getGlTextureId() : 0;
	ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
	ImGui::SameLine();

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

void MaterialNode::onGraphPropertyChanged(t_graph_property_id id)
{
	if (m_materialArrayProperty && m_materialArrayProperty->getId() == id)
	{
		auto textureArray = m_materialArrayProperty->getArray();
		auto it = std::find(textureArray.begin(), textureArray.end(), m_sourceProperty);

		if (it == textureArray.end())
		{
			setMaterialSource(GraphMaterialPropertyPtr());
		}
	}
}

// -- MaterialNode Factory -----
NodePtr MaterialNodeFactory::createNode(const NodeEditorState* editorState) const
{
	// Create the node and pins
	MaterialNodePtr node = std::make_shared<MaterialNode>();
	MaterialPinPtr outputPin = node->addPin<MaterialPin>("material", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}