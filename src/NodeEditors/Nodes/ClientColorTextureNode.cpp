#include "ClientColorTextureNode.h"
#include "GlScopedObjectBinding.h"
#include "GlFrameCompositor.h"
#include "GlFrameBuffer.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlShaderCache.h"
#include "GlStateStack.h"
#include "GlTexture.h"
#include "GlTextureCache.h"
#include "GlTriangulatedMesh.h"
#include "Logger.h"
#include "MainWindow.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"
#include "StringUtils.h"

#include "DataSources/ClientListDataSource.h"

#include "Graphs/NodeEvaluator.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/CompositorNodeGraph.h"

#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"

#include "Properties/GraphTextureProperty.h"

#include "imgui.h"
#include "imnodes.h"

// -- ClientTextureNodeConfig -----
configuru::Config ClientColorTextureNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["client_texture_type"] = k_clientColorTextureTypeStrings[(int)clientTextureType];
	pt["client_index"] = clientIndex;

	return pt;
}

void ClientColorTextureNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	const std::string clientTextureTypeString =
		pt.get_or<std::string>(
			"client_texture_type",
			k_clientColorTextureTypeStrings[(int)eClientColorTextureType::colorRGB]);
	clientTextureType =
		StringUtils::FindEnumValue<eClientColorTextureType>(
			clientTextureTypeString, k_clientColorTextureTypeStrings);

	clientIndex= pt.get_or<int>("client_index", 0);
}

// -- ClientTextureNode -----
bool ClientColorTextureNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto clientTextureNodeConfig = std::static_pointer_cast<const ClientColorTextureNodeConfig>(nodeConfig);

		m_clientTextureType= clientTextureNodeConfig->clientTextureType;
		m_clientIndex= clientTextureNodeConfig->clientIndex;
		return true;
	}

	return false;
}

void ClientColorTextureNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto clientTextureNodeConfig = std::static_pointer_cast<ClientColorTextureNodeConfig>(nodeConfig);
	clientTextureNodeConfig->clientTextureType = m_clientTextureType;
	clientTextureNodeConfig->clientIndex = m_clientIndex;

	Node::saveToConfig(nodeConfig);
}

GlTexturePtr ClientColorTextureNode::getTextureResource() const
{
	GlFrameCompositor* compositor= MainWindow::getInstance()->getFrameCompositor();
	if (compositor != nullptr)
	{
		GlTexturePtr clientTexture= compositor->getClientColorSourceTexture(m_clientIndex, m_clientTextureType);

		// If the client texture is not available, return a black texture
		if (clientTexture)
		{
			return clientTexture;
		}
		else
		{
			auto* textureCache = getOwnerGraph()->getOwnerWindow()->getTextureCache();

			if (m_clientTextureType == eClientColorTextureType::colorRGB)
			{
				return textureCache->tryGetTextureByName(INTERNAL_TEXTURE_BLACK_RGB);
			}
			else if (m_clientTextureType == eClientColorTextureType::colorRGBA)
			{
				return textureCache->tryGetTextureByName(INTERNAL_TEXTURE_BLACK_RGBA);
			}
		}
	}

	return GlTexturePtr();
}

bool ClientColorTextureNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Since the frame compositor can change the client source texture can change out from under us
	// it's safest to just refresh the output texture pin every frame
	auto outputPin= getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT);

	GlTexturePtr clientTexture= getTextureResource();
	outputPin->setValue(clientTexture);

	return true;
}

void ClientColorTextureNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

std::string ClientColorTextureNode::editorGetTitle() const
{
	if (!isDefaultNode())
	{ 
		return StringUtils::stringify("Client Source ", m_clientIndex);
	}

	return "Client Source";
}

void ClientColorTextureNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture Preview
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	GlTexturePtr textureResource = getTextureResource();
	uint32_t glTextureId = textureResource ? textureResource->getGlTextureId() : 0;
	ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
	ImGui::SameLine();

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

void ClientColorTextureNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Client Texture Node"))
	{
		// Texture Type
		int iTextureType= (int)m_clientTextureType;
		if (NodeEditorUI::DrawSimpleComboBoxProperty(
			"clientTextureType",
			"Type",
			"colorRGB\0colorRGBA\0",
			iTextureType))
		{
			m_clientTextureType= (eClientColorTextureType)iTextureType;
		}

		// Texture Type
		ClientListDataSource dataSource;
		NodeEditorUI::DrawComboBoxProperty(
			"clientSourceIndex",
			"Source",
			&dataSource,
			m_clientIndex);
	}
}

// -- ClientTextureNode Factory -----
NodePtr ClientColorTextureNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node = NodeFactory::createNode(editorState);
	auto outputPin = node->addPin<TexturePin>("texture", eNodePinDirection::OUTPUT);
	outputPin->editorSetShowPinName(false);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}