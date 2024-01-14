#include "App.h"
#include "GlFrameCompositor.h"
#include "GlTexture.h"
#include "Logger.h"
#include "MainWindow.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"
#include "StringUtils.h"
#include "VideoTextureNode.h"

#include "Graphs/NodeGraph.h"

#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"

#include "Properties/GraphTextureProperty.h"

#include "imgui.h"
#include "imnodes.h"

// -- VideoTextureNodeConfig -----
configuru::Config VideoTextureNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["video_texture_source"] = k_videoTextureStrings[(int)videoTextureSource];

	return pt;
}

void VideoTextureNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	const std::string textureSourceString =
		pt.get_or<std::string>(
			"video_texture_source",
			k_videoTextureStrings[(int)eVideoTextureSource::video_texture]);
	videoTextureSource = 
		StringUtils::FindEnumValue<eVideoTextureSource>(
			textureSourceString, k_videoTextureStrings);
}

// -- VideoTextureNode -----
bool VideoTextureNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto videoTextureNodeConfig = std::static_pointer_cast<const VideoTextureNodeConfig>(nodeConfig);

		m_videoTextureSource= videoTextureNodeConfig->videoTextureSource;
		return true;
	}

	return false;
}

void VideoTextureNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto videoTextureNodeConfig = std::static_pointer_cast<VideoTextureNodeConfig>(nodeConfig);
	videoTextureNodeConfig->videoTextureSource = m_videoTextureSource;

	Node::saveToConfig(nodeConfig);
}

GlTexturePtr VideoTextureNode::getTextureResource() const
{
	GlFrameCompositor* compositor= MainWindow::getInstance()->getFrameCompositor();
	if (compositor != nullptr)
	{
		return compositor->getVideoSourceTexture(m_videoTextureSource);
	}

	return GlTexturePtr();
}

bool VideoTextureNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Since the frame compositor can change the video source texture can change out from under us
	// it's safest to just refresh the output texture pin every frame
	auto outputPin= getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT);
	outputPin->setValue(getTextureResource());
	outputPin->editorSetShowPinName(false);

	return true;
}

void VideoTextureNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

void VideoTextureNode::editorRenderNode(const NodeEditorState& editorState)
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

void VideoTextureNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Video Texture Node"))
	{
		// Texture Source
		int iTextureSource = (int)m_videoTextureSource;
		if (NodeEditorUI::DrawSimpleComboBoxProperty(
			"videoTextureNodeSource",
			"Source",
			"Video\0Distortion\0",
			iTextureSource))
		{
			m_videoTextureSource = (eVideoTextureSource)iTextureSource;
		}
	}
}

// -- VideoTextureNode Factory -----
NodePtr VideoTextureNodeFactory::createNode(const NodeEditorState& editorState) const
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