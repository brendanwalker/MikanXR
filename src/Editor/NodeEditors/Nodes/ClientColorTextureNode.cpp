#include "ClientColorTextureNode.h"
#include "MkScopedObjectBinding.h"
#include "GlFrameCompositor.h"
#include "IMkFrameBuffer.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MikanShaderCache.h"
#include "MkStateStack.h"
#include "IMkTexture.h"
#include "MikanTextureCache.h"
#include "IMkTriangulatedMesh.h"
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
	pt["vertical_flip"] = bVerticalFlip;

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
	bVerticalFlip = pt.get_or<bool>("vertical_flip", false);

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
		m_bVerticalFlip= clientTextureNodeConfig->bVerticalFlip;

		return true;
	}

	return false;
}

void ClientColorTextureNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto clientTextureNodeConfig = std::static_pointer_cast<ClientColorTextureNodeConfig>(nodeConfig);
	clientTextureNodeConfig->clientTextureType = m_clientTextureType;
	clientTextureNodeConfig->clientIndex = m_clientIndex;
	clientTextureNodeConfig->bVerticalFlip = m_bVerticalFlip;

	Node::saveToConfig(nodeConfig);
}

IMkTexturePtr ClientColorTextureNode::getTextureResource() const
{
	return 
		m_bVerticalFlip && m_colorFrameBuffer 
		? m_colorFrameBuffer->getColorTexture() 
		: getClientColorSourceTexture();
}

bool ClientColorTextureNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Since the frame compositor can change the client source texture can change out from under us
	// it's safest to just refresh the output texture pin every frame
	auto outputPin= getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT);

	// Render the color texture to the frame buffer if we want to flip the Y axis
	if (m_bVerticalFlip)
	{
		updateColorFrameBuffer(evaluator, getClientColorSourceTexture());
	}

	// Render the output color texture to the output pin
	outputPin->setValue(getTextureResource());

	return true;
}

IMkTexturePtr ClientColorTextureNode::getClientColorSourceTexture() const
{
	GlFrameCompositor* compositor = MainWindow::getInstance()->getFrameCompositor();
	if (compositor != nullptr)
	{
		IMkTexturePtr clientTexture = compositor->getClientColorSourceTexture(m_clientIndex, m_clientTextureType);

		// If the client texture is not available, return a black texture
		if (clientTexture)
		{
			return clientTexture;
		}
		else
		{
			auto* textureCache = getOwnerGraph()->getOwnerWindow()->getTextureCache();

			if (m_clientTextureType == eClientColorTextureType::colorRGBA)
			{
				return textureCache->tryGetTextureByName(INTERNAL_TEXTURE_BLACK_RGBA);
			}
			else
			{
				return textureCache->tryGetTextureByName(INTERNAL_TEXTURE_BLACK_RGB);
			}
		}
	}

	return IMkTexturePtr();
}

void ClientColorTextureNode::updateColorFrameBuffer(NodeEvaluator& evaluator, IMkTexturePtr clientTexture)
{
	IMkWindow* ownerWindow = evaluator.getCurrentWindow();

	assert(m_clientTextureType == eClientColorTextureType::colorRGBA || 
		   m_clientTextureType == eClientColorTextureType::colorRGB);

	// Create the color frame buffer if it doesn't exist yet and we want to flip the Y axis
	if (m_colorFrameBuffer == nullptr && m_bVerticalFlip)
	{
		m_colorFrameBuffer = createMkFrameBuffer("ClientColorTextureNode");
		m_colorFrameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR);

		switch (m_clientTextureType)
		{
		case eClientColorTextureType::colorRGB:
			m_colorFrameBuffer->setColorFormat(IMkFrameBuffer::eColorFormat::RGB);
			break;
		case eClientColorTextureType::colorRGBA:
			m_colorFrameBuffer->setColorFormat(IMkFrameBuffer::eColorFormat::RGBA);
			break;
		}
	}
	// Dispose the color frame buffer if it exists and we don't want to flip the Y axis
	else if (m_colorFrameBuffer != nullptr && !m_bVerticalFlip)
	{
		m_colorFrameBuffer->disposeResources();
		m_colorFrameBuffer = nullptr;
		m_colorMaterialInstance = nullptr;
	}

	// Update the color frame buffer if it exists
	if (m_colorFrameBuffer)
	{
		// Update render target size
		m_colorFrameBuffer->setSize(clientTexture->getTextureWidth(), clientTexture->getTextureHeight());

		// Update render resources if the frame buffer is not valid
		if (!m_colorFrameBuffer->isValid())
		{
			// Re-create the frame buffer if it's not valid
			m_colorFrameBuffer->createResources();

			// Re-create the render material instance
			const std::string colorMaterialName =
				m_clientTextureType == eClientColorTextureType::colorRGBA
				? INTERNAL_MATERIAL_PT_FULLSCREEN_RGBA_TEXTURE
				: INTERNAL_MATERIAL_PT_FULLSCREEN_RGB_TEXTURE;
			MkMaterialConstPtr colorMaterial =
				ownerWindow->getShaderCache()->getMaterialByName(colorMaterialName);
			if (colorMaterial != nullptr)
			{
				m_colorMaterialInstance = std::make_shared<GlMaterialInstance>(colorMaterial);
			}
			else
			{
				m_colorMaterialInstance = nullptr;
				MIKAN_LOG_ERROR("updateColorFrameBuffer") << "Failed to get color material";
			}
		}
	}

	// Render the color texture to the frame buffer
	if (m_bVerticalFlip && m_colorMaterialInstance)
	{
		MkScopedObjectBinding colorFramebufferBinding(
			*ownerWindow->getMkStateStack().getCurrentState(),
			"Color Texture Framebuffer Scope",
			m_colorFrameBuffer);
		if (colorFramebufferBinding)
		{
			IMkStatePtr glState = colorFramebufferBinding.getGlState();

			evaluateFlippedColorTexture(glState, clientTexture);
		}
	}
}

void ClientColorTextureNode::evaluateFlippedColorTexture(IMkStatePtr glState, IMkTexturePtr colorTexture)
{
	assert(colorTexture);
	assert(m_colorMaterialInstance);

	MkMaterialConstPtr material = m_colorMaterialInstance->getMaterial();
	if (auto materialBinding = material->bindMaterial())
	{
		// Bind the color texture
		if (m_clientTextureType == eClientColorTextureType::colorRGBA)
		{
			m_colorMaterialInstance->setTextureBySemantic(eUniformSemantic::rgbaTexture, colorTexture);
		}
		else
		{
			m_colorMaterialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, colorTexture);
		}

		// Draw the color texture
		if (auto materialInstanceBinding = m_colorMaterialInstance->bindMaterialInstance(materialBinding))
		{
			auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());

			compositorGraph->getLayerVFlippedMesh()->drawElements();
		}
	}
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
		return StringUtils::stringify("Client Color ", m_clientIndex);
	}

	return "Client Color";
}

void ClientColorTextureNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture Preview
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	IMkTexturePtr textureResource = getTextureResource();
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

		// Vertical Flip
		NodeEditorUI::DrawCheckBoxProperty(
			"drawColorTextureVerticalFlip",
			"Vertical Flip",
			m_bVerticalFlip);
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