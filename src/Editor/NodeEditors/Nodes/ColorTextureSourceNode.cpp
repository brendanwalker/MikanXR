#include "ColorTextureSourceNode.h"
#include "CameraComponent.h"
#include "MkScopedObjectBinding.h"
#include "IMkFrameBuffer.h"
#include "IMkWindow.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MikanShaderCache.h"
#include "MikanCoreTypes.h"
#include "MkStateStack.h"
#include "IMkTexture.h"
#include "MikanTextureCache.h"
#include "IMkTriangulatedMesh.h"
#include "Logger.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"
#include "ProjectManager.h"
#include "StringUtils.h"
#include "TextureSourceQueries.h"
#include "TextureSourceComponent.h"

#include "DataSources/TextureSourceListDataSource.h"

#include "Graphs/NodeEvaluator.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/CompositorNodeGraph.h"

#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"

#include "Properties/GraphTextureProperty.h"

#include "imgui.h"
#include "imnodes.h"

// -- ClientTextureNodeConfig -----
configuru::Config ColorTextureSourceNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["texture_source_color_type"] = k_textureSourceColorTypeStrings[(int)textureSourceColorType];
	pt["texture_source_id"] = textureSourceId;
	pt["vertical_flip"] = bVerticalFlip;

	return pt;
}

void ColorTextureSourceNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	const std::string clientTextureTypeString =
		pt.get_or<std::string>(
			"texture_source_color_type",
			k_textureSourceColorTypeStrings[(int)eTextureSourceColorType::colorRGB]);
	textureSourceColorType =
		StringUtils::FindEnumValue<eTextureSourceColorType>(
			clientTextureTypeString, k_textureSourceColorTypeStrings);
	bVerticalFlip = pt.get_or<bool>("vertical_flip", false);

	textureSourceId = pt.get_or<int>("texture_source_id", INVALID_MIKAN_ID);
}

// -- ClientTextureNode -----
bool ColorTextureSourceNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto textureSourceNodeConfig = std::static_pointer_cast<const ColorTextureSourceNodeConfig>(nodeConfig);

		m_clientTextureType= textureSourceNodeConfig->textureSourceColorType;
		m_bVerticalFlip= textureSourceNodeConfig->bVerticalFlip;

		// Get the client video source component corresponding to the saved video source id
		ProjectManagerPtr projectManager = getOwnerProject();
		if (projectManager)
		{
			m_textureSourceComponent =
				TextureSourceQueries::getTextureSourceById(
					projectManager,
					textureSourceNodeConfig->textureSourceId);
		}

		return true;
	}

	return false;
}

void ColorTextureSourceNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto textureSourceNodeConfig = std::static_pointer_cast<ColorTextureSourceNodeConfig>(nodeConfig);
	TextureSourceComponentPtr textureSourceComponent= getTextureSourceComponent();

	textureSourceNodeConfig->textureSourceColorType = m_clientTextureType;
	textureSourceNodeConfig->bVerticalFlip = m_bVerticalFlip;
	textureSourceNodeConfig->textureSourceId =
		textureSourceComponent 
		? textureSourceComponent->getTextureSourceId() 
		: INVALID_MIKAN_ID;

	Node::saveToConfig(nodeConfig);
}

TextureSourceComponentPtr ColorTextureSourceNode::getTextureSourceComponent() const
{
	return m_textureSourceComponent.lock();
}

IMkTexturePtr ColorTextureSourceNode::getTextureResource() const
{
	return 
		m_bVerticalFlip && m_colorFrameBuffer 
		? m_colorFrameBuffer->getColorTexture() 
		: getColorSourceTexture();
}

bool ColorTextureSourceNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Since the frame compositor can change the client source texture can change out from under us
	// it's safest to just refresh the output texture pin every frame
	auto outputPin= getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT);

	// Render the color texture to the frame buffer if we want to flip the Y axis
	if (m_bVerticalFlip)
	{
		updateColorFrameBuffer(evaluator, getColorSourceTexture());
	}

	// Render the output color texture to the output pin
	outputPin->setValue(getTextureResource());

	return true;
}

IMkTexturePtr ColorTextureSourceNode::getColorSourceTexture() const
{
	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());
	CameraComponentPtr boundCameraComponent= compositorGraph->getBoundCameraComponent();
	TextureSourceComponentPtr textureSourceComponent = getTextureSourceComponent();

	if (boundCameraComponent && textureSourceComponent)
	{
		IMkTexturePtr clientTexture = 
			textureSourceComponent->getClientColorSourceTexture(
				boundCameraComponent->getCameraId(),
				m_clientTextureType);

		// If the client texture is not available, return a black texture
		if (clientTexture)
		{
			return clientTexture;
		}
		else
		{
			auto* textureCache = getOwnerGraph()->getOwnerWindow()->getTextureCache();

			if (m_clientTextureType == eTextureSourceColorType::colorRGBA)
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

void ColorTextureSourceNode::updateColorFrameBuffer(NodeEvaluator& evaluator, IMkTexturePtr clientTexture)
{
	IMkWindow* ownerWindow = evaluator.getCurrentWindow();

	assert(m_clientTextureType == eTextureSourceColorType::colorRGBA || 
		   m_clientTextureType == eTextureSourceColorType::colorRGB);

	// Create the color frame buffer if it doesn't exist yet and we want to flip the Y axis
	if (m_colorFrameBuffer == nullptr && m_bVerticalFlip)
	{
		m_colorFrameBuffer = createMkFrameBuffer("ColorTextureSourceNode");
		m_colorFrameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR);

		switch (m_clientTextureType)
		{
		case eTextureSourceColorType::colorRGB:
			m_colorFrameBuffer->setColorFormat(IMkFrameBuffer::eColorFormat::RGB);
			break;
		case eTextureSourceColorType::colorRGBA:
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
				m_clientTextureType == eTextureSourceColorType::colorRGBA
				? INTERNAL_MATERIAL_PT_FULLSCREEN_RGBA_TEXTURE
				: INTERNAL_MATERIAL_PT_FULLSCREEN_RGB_TEXTURE;
			MkMaterialConstPtr colorMaterial =
				ownerWindow->getShaderCache()->getMaterialByName(colorMaterialName);
			if (colorMaterial != nullptr)
			{
				m_colorMaterialInstance = createMkMaterialInstance(colorMaterial);
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
			ownerWindow->getMkStateStack().getCurrentState(),
			"Color Texture Framebuffer Scope",
			m_colorFrameBuffer);
		if (colorFramebufferBinding)
		{
			IMkState* glState = colorFramebufferBinding.getMkState();

			evaluateFlippedColorTexture(glState, clientTexture);
		}
	}
}

void ColorTextureSourceNode::evaluateFlippedColorTexture(IMkState* glState, IMkTexturePtr colorTexture)
{
	assert(colorTexture);
	assert(m_colorMaterialInstance);

	MkMaterialConstPtr material = m_colorMaterialInstance->getMaterial();
	if (auto materialBinding = material->bindMaterial())
	{
		// Bind the color texture
		if (m_clientTextureType == eTextureSourceColorType::colorRGBA)
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

void ColorTextureSourceNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

std::string ColorTextureSourceNode::editorGetTitle() const
{
	if (!isDefaultNode())
	{ 
		std::string sourceId = getTextureSourceComponent()->getName();

		return StringUtils::stringify("Color Source ", sourceId);
	}

	return "Color Source";
}

void ColorTextureSourceNode::editorRenderNode(const NodeEditorState& editorState)
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

void ColorTextureSourceNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Client Texture Node"))
	{
		// Texture Type
		int iTextureType= (int)m_clientTextureType;
		if (NodeEditorUI::DrawSimpleComboBoxProperty(
			"textureSourceColorType",
			"Type",
			"colorRGB\0colorRGBA\0",
			iTextureType))
		{
			m_clientTextureType= (eTextureSourceColorType)iTextureType;
		}

		// Texture Type
		ProjectManagerPtr projectManager = getOwnerProject();
		TextureSourceListDataSource dataSource(projectManager);
		if (dataSource.getEntryCount() > 0)
		{
			TextureSourceComponentPtr TextureSourceComponent = getTextureSourceComponent();

			int selectedIndex = dataSource.getEntryIndex(TextureSourceComponent);
			NodeEditorUI::DrawComboBoxProperty(
				"textureSourceIndex",
				"Source",
				&dataSource,
				selectedIndex);
			m_textureSourceComponent = dataSource.getEntryAtIndex(selectedIndex);
		}

		// Vertical Flip
		NodeEditorUI::DrawCheckBoxProperty(
			"drawColorTextureVerticalFlip",
			"Vertical Flip",
			m_bVerticalFlip);
	}
}

// -- ClientTextureNode Factory -----
NodePtr ColorTextureSourceNodeFactory::createNode(const NodeEditorState& editorState) const
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