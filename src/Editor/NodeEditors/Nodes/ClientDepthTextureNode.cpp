#include "ClientDepthTextureNode.h"
#include "GlScopedObjectBinding.h"
#include "GlFrameCompositor.h"
#include "GlFrameBuffer.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlShaderCache.h"
#include "GlStateStack.h"
#include "IMkTexture.h"
#include "MikanTextureCache.h"
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
configuru::Config ClientDepthTextureNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["client_texture_type"] = k_clientDepthTextureTypeStrings[(int)clientTextureType];
	pt["client_index"] = clientIndex;
	pt["vertical_flip"] = bVerticalFlip;

	return pt;
}

void ClientDepthTextureNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	const std::string clientTextureTypeString =
		pt.get_or<std::string>(
			"client_texture_type",
			k_clientDepthTextureTypeStrings[(int)eClientDepthTextureType::depthPackRGBA]);
	clientTextureType =
		StringUtils::FindEnumValue<eClientDepthTextureType>(
			clientTextureTypeString, k_clientDepthTextureTypeStrings);
	bVerticalFlip = pt.get_or<bool>("vertical_flip", false);

	clientIndex= pt.get_or<int>("client_index", 0);
}

// -- ClientTextureNode -----
bool ClientDepthTextureNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto clientTextureNodeConfig = std::static_pointer_cast<const ClientDepthTextureNodeConfig>(nodeConfig);

		m_clientTextureType= clientTextureNodeConfig->clientTextureType;
		m_clientIndex= clientTextureNodeConfig->clientIndex;
		m_bVerticalFlip= clientTextureNodeConfig->bVerticalFlip;

		return true;
	}

	return false;
}

void ClientDepthTextureNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto clientTextureNodeConfig = std::static_pointer_cast<ClientDepthTextureNodeConfig>(nodeConfig);
	clientTextureNodeConfig->clientTextureType = m_clientTextureType;
	clientTextureNodeConfig->clientIndex = m_clientIndex;
	clientTextureNodeConfig->bVerticalFlip = m_bVerticalFlip;

	Node::saveToConfig(nodeConfig);
}

GlTexturePtr ClientDepthTextureNode::getTextureResource() const
{
	return m_linearDepthFrameBuffer ? m_linearDepthFrameBuffer->getDepthTexture() : GlTexturePtr();
}

bool ClientDepthTextureNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Since the frame compositor can change the client source texture can change out from under us
	// it's safest to just refresh the output texture pin every frame
	auto outputPin= getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT);

	// Update the linear depth texture from the client depth texture
	GlTexturePtr clientDepthTexture= getClientDepthSourceTexture();
	updateLinearDepthFrameBuffer(evaluator, clientDepthTexture);

	// Return the linear depth texture from the frame buffer
	GlTexturePtr linearDepthTexture= getTextureResource();
	outputPin->setValue(linearDepthTexture);

	return true;
}

GlTexturePtr ClientDepthTextureNode::getClientDepthSourceTexture() const
{
	GlFrameCompositor* compositor = MainWindow::getInstance()->getFrameCompositor();
	if (compositor != nullptr)
	{
		GlTexturePtr clientTexture = compositor->getClientDepthSourceTexture(m_clientIndex, m_clientTextureType);

		// If the client texture is not available, return a black texture
		if (clientTexture)
		{
			return clientTexture;
		}
		else
		{
			auto* textureCache = getOwnerGraph()->getOwnerWindow()->getTextureCache();

			if (m_clientTextureType == eClientDepthTextureType::depthPackRGBA )
			{
				return textureCache->tryGetTextureByName(INTERNAL_TEXTURE_BLACK_RGBA);
			}
		}
	}

	return GlTexturePtr();
}

void ClientDepthTextureNode::updateLinearDepthFrameBuffer(NodeEvaluator& evaluator, GlTexturePtr clientTexture)
{
	assert(m_clientTextureType == eClientDepthTextureType::depthPackRGBA);

	if (m_linearDepthFrameBuffer == nullptr)
	{
		m_linearDepthFrameBuffer = std::make_shared<GlFrameBuffer>("ClientDepthTextureNode");
		m_linearDepthFrameBuffer->setFrameBufferType(GlFrameBuffer::eFrameBufferType::COLOR_AND_DEPTH);
	}

	// Update render target size
	m_linearDepthFrameBuffer->setSize(clientTexture->getTextureWidth(), clientTexture->getTextureHeight());
	if (!m_linearDepthFrameBuffer->isValid())
	{
		m_linearDepthFrameBuffer->createResources();
	}

	IGlWindow* ownerWindow= evaluator.getCurrentWindow();
	GlScopedObjectBinding depthFramebufferBinding(
		*ownerWindow->getGlStateStack().getCurrentState(),
		"Depth Texture Framebuffer Scope",
		m_linearDepthFrameBuffer);
	if (depthFramebufferBinding)
	{
		GlState& glState = depthFramebufferBinding.getGlState();
		GlMaterialConstPtr depthUnpackMaterial =
			ownerWindow->getShaderCache()->getMaterialByName(
				INTERNAL_MATERIAL_UNPACK_RGBA_DEPTH_TEXTURE);

		if (depthUnpackMaterial != nullptr)
		{
			m_depthMaterialInstance = std::make_shared<GlMaterialInstance>(depthUnpackMaterial);
		}

		if (m_depthMaterialInstance)
		{
			evaluateDepthTexture(glState, clientTexture);
		}
	}
}

void ClientDepthTextureNode::evaluateDepthTexture(GlState& glState, GlTexturePtr depthTexture)
{
	assert(depthTexture);
	assert(m_depthMaterialInstance);

	GlMaterialConstPtr material = m_depthMaterialInstance->getMaterial();
	if (auto materialBinding = material->bindMaterial())
	{
		// Bind the depth texture
		m_depthMaterialInstance->setTextureBySemantic(eUniformSemantic::rgbaTexture, depthTexture);

		if (auto materialInstanceBinding = m_depthMaterialInstance->bindMaterialInstance(materialBinding))
		{
			auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());

			if (m_bVerticalFlip)
			{
				compositorGraph->getLayerVFlippedMesh()->drawElements();
			}
			else
			{
				compositorGraph->getLayerMesh()->drawElements();
			}
		}
	}
}

void ClientDepthTextureNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

std::string ClientDepthTextureNode::editorGetTitle() const
{
	if (!isDefaultNode())
	{ 
		return StringUtils::stringify("Client Depth ", m_clientIndex);
	}

	return "Client Depth";
}

void ClientDepthTextureNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture Preview (color texture of the frame buffer)
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	GlTexturePtr colorTexture =
		m_linearDepthFrameBuffer ? m_linearDepthFrameBuffer->getColorTexture() : GlTexturePtr();
	uint32_t glTextureId =
		colorTexture ? colorTexture->getGlTextureId() : 0;
	ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
	ImGui::SameLine();

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

void ClientDepthTextureNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Client Texture Node"))
	{
		// Texture Type
		int iTextureType= (int)m_clientTextureType;
		if (NodeEditorUI::DrawSimpleComboBoxProperty(
			"clientTextureType",
			"Type",
			"depthPackRGBA\0",
			iTextureType))
		{
			m_clientTextureType= (eClientDepthTextureType)iTextureType;
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
			"drawDepthTextureVerticalFlip",
			"Vertical Flip",
			m_bVerticalFlip);
	}
}

// -- ClientTextureNode Factory -----
NodePtr ClientDepthTextureNodeFactory::createNode(const NodeEditorState& editorState) const
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