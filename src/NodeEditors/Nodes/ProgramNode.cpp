#include "ProgramNode.h"
#include "GlProgram.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

#include <typeinfo>
#include <GL/glew.h>

ProgramNode::ProgramNode()
	: Node()
	, m_attachmentsPinsStartId(1)
	, m_dispatchType(ProgramDispatchType::ARRAY)
	, m_drawMode(GL_POINTS)
{
	m_dispatchSize[3] = {};
}

ProgramNode::ProgramNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
	, m_attachmentsPinsStartId(1)
	, m_dispatchType(ProgramDispatchType::ARRAY)
	, m_drawMode(GL_POINTS)
{

}

std::string ProgramNode::editorGetTitle() const
{ 
	return m_target ? m_target->getProgramCode().getProgramName() : "Program";
}