#include "ClientTextureNode.h"
#include "GlFrameCompositor.h"
#include "GlTexture.h"
#include "GlTextureCache.h"
#include "Logger.h"
#include "MainWindow.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"
#include "StringUtils.h"

#include "Graphs/NodeGraph.h"

#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"

#include "Properties/GraphTextureProperty.h"

#include "imgui.h"
#include "imnodes.h"

// -- ClientListDataSource ---
class ClientListDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	ClientListDataSource()
	{
		GlFrameCompositor* compositor = MainWindow::getInstance()->getFrameCompositor();
		if (compositor != nullptr)
		{
			auto& clientSources= compositor->getClientSources();

			for (auto it = clientSources.getMap().begin(); it != clientSources.getMap().end(); it++)
			{
				GlFrameCompositor::ClientSource* clientSource= it->second;

				comboEntries.push_back({clientSource->clientSourceIndex, clientSource->clientId});
			}
		}

		if (comboEntries.size() == 0)
		{
			comboEntries.push_back({0, "Client 0"});
		}
	}

	inline int getClientIndex(int index)
	{
		return comboEntries[index].clientIndex;
	}

	virtual int getEntryCount() override
	{
		return (int)comboEntries.size();
	}

	virtual const std::string& getEntryDisplayString(int index) override
	{
		return comboEntries[index].clientEntryName;
	}

private:
	struct ComboEntry
	{
		int clientIndex;
		std::string clientEntryName;
	};

	std::vector<ComboEntry> comboEntries;
};

// -- ClientTextureNodeConfig -----
configuru::Config ClientTextureNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["client_texture_type"] = k_clientTextureTypeStrings[(int)clientTextureType];
	pt["client_index"] = clientIndex;

	return pt;
}

void ClientTextureNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	const std::string clientTextureTypeString =
		pt.get_or<std::string>(
			"client_texture_type",
			k_clientTextureTypeStrings[(int)eClientTextureType::colorRGB]);
	clientTextureType =
		StringUtils::FindEnumValue<eClientTextureType>(
			clientTextureTypeString, k_clientTextureTypeStrings);

	clientIndex= pt.get_or<int>("client_index", 0);
}

// -- ClientTextureNode -----
bool ClientTextureNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto clientTextureNodeConfig = std::static_pointer_cast<const ClientTextureNodeConfig>(nodeConfig);

		m_clientTextureType= clientTextureNodeConfig->clientTextureType;
		m_clientIndex= clientTextureNodeConfig->clientIndex;
		return true;
	}

	return false;
}

void ClientTextureNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto clientTextureNodeConfig = std::static_pointer_cast<ClientTextureNodeConfig>(nodeConfig);
	clientTextureNodeConfig->clientTextureType = m_clientTextureType;
	clientTextureNodeConfig->clientIndex = m_clientIndex;

	Node::saveToConfig(nodeConfig);
}

GlTexturePtr ClientTextureNode::getTextureResource() const
{
	GlFrameCompositor* compositor= MainWindow::getInstance()->getFrameCompositor();
	if (compositor != nullptr)
	{
		GlTexturePtr clientTexture= compositor->getClientSourceTexture(m_clientIndex, m_clientTextureType);

		// If the client texture is not available, return a black texture
		if (clientTexture)
		{
			return clientTexture;
		}
		else
		{
			auto* textureCache = getOwnerGraph()->getOwnerWindow()->getTextureCache();

			if (m_clientTextureType == eClientTextureType::colorRGB)
			{
				return textureCache->tryGetTextureByName(INTERNAL_TEXTURE_BLACK_RGB);
			}
			else if (m_clientTextureType == eClientTextureType::colorRGBA ||
					 m_clientTextureType == eClientTextureType::depthPackRGBA)
			{
				return textureCache->tryGetTextureByName(INTERNAL_TEXTURE_BLACK_RGBA);
			}
		}
	}

	return GlTexturePtr();
}

bool ClientTextureNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Since the frame compositor can change the client source texture can change out from under us
	// it's safest to just refresh the output texture pin every frame
	auto outputPin= getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT);
	outputPin->setValue(getTextureResource());

	return true;
}

void ClientTextureNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

std::string ClientTextureNode::editorGetTitle() const
{
	if (!isDefaultNode())
	{ 
		if (m_clientTextureType != eClientTextureType::INVALID)
		{
			return
				StringUtils::stringify(
					"Client Source ", m_clientIndex,
					" (", k_clientTextureTypeStrings[(int)m_clientTextureType], ")");
		}
		else
		{
			return StringUtils::stringify("Client Source ", m_clientIndex, " (INVALID)");
		}
	}

	return "Client Source";
}

void ClientTextureNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture
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

void ClientTextureNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Client Texture Node"))
	{
		// Texture Type
		int iTextureType= (int)m_clientTextureType;
		if (NodeEditorUI::DrawSimpleComboBoxProperty(
			"clientTextureType",
			"Type",
			"colorRGB\0colorRGBA\0depthPackRGBA\0",
			iTextureType))
		{
			m_clientTextureType= (eClientTextureType)iTextureType;
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
NodePtr ClientTextureNodeFactory::createNode(const NodeEditorState& editorState) const
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