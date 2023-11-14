#include "EditorNode.h"
#include "GlCommon.h"

// -- EditorNode -----
EditorNode::EditorNode()
	: id(-1)
	, type(EditorNodeType::NODE)
{
	nodePos.x = nodePos.y = 0.f;
}

// -- EditorEventNode -----
EditorEventNode::EditorEventNode()
	: eventNodeType(EditorEventNodeType::INIT)
{
}

// -- EditorProgramNode -----
EditorProgramNode::EditorProgramNode()
	: attachmentsPinsStartId(1)
	, dispatchType(EditorProgramDispatchType::ARRAY)
	, drawMode(GL_POINTS)
{
	dispatchSize[3]= {};
}

// -- EditorImageNode -----
EditorImageNode::EditorImageNode()
	: sizeX(0)
	, sizeY(0)
	, texture(-1)
{
};

EditorImageNode::~EditorImageNode()
{
	if (texture != -1)
		glDeleteTextures(1, &texture);
}

// -- EditorBlockNode -----
EditorBlockNode::EditorBlockNode()
	: size(0)
	, ubo(-1)
	, ssbo(-1)
	, ssboSize(1)
{
}

EditorBlockNode::~EditorBlockNode()
{
	if (ubo != -1)
		glDeleteBuffers(1, &ubo);
	if (ssbo != -1)
		glDeleteBuffers(1, &ssbo);
}

// -- EditorPingPongNode -----
EditorPingPongNode::EditorPingPongNode()
	: pingpongType(EditorPingPongNodeType::BUFFER)
	, size(0)
{
}