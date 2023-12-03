#pragma once

#include "NodeFwd.h"
#include "glm/ext/vector_float2.hpp"

#include <string>
#include <vector>
#include <stdint.h>

struct NodeDimensions
{
	float titleWidth= 0.f;
	float inputColomnWidth= 0.f;
	float outputColomnWidth= 0.f;
	float totalNodeWidth= 0.f;
};

class Node
{
public:
	Node();
	Node(NodeGraphPtr ownerGraph);

	inline int getId() const { return m_id; }
	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }
	inline const glm::vec2& getNodePos() const { return m_nodePos; }
	inline const std::vector<NodePinPtr>& getInputPins() const { return m_pinsIn; }
	inline const std::vector<NodePinPtr>& getOutputPins() const { return m_pinsOut; }
	inline void setNodePos(const glm::vec2& nodePos) { m_nodePos= nodePos; }

	bool disconnectPin(NodePinPtr pinPtr);
	void disconnectAllPins();

	virtual std::string editorGetTitle() const { return "Node"; }
	virtual bool editorCanDelete() const { return true; }
	virtual void editorRender(class NodeEditorState* editorState);

protected:
	virtual void editorRenderTitle(class NodeEditorState* editorState) const;

	virtual void editorComputeNodeDimensions(NodeDimensions& outDims) const;
	virtual void editorRenderPushNodeStyle(NodeEditorState* editorState) const;
	virtual void editorRenderPopNodeStyle(NodeEditorState* editorState) const;
	virtual void editorRenderInputPins(NodeEditorState* editorState) const;
	virtual void editorRenderOutputPins(NodeEditorState* editorState) const;

protected:
	int m_id;
	NodeGraphPtr m_ownerGraph;
	std::vector<NodePinPtr> m_pinsIn;
	std::vector<NodePinPtr> m_pinsOut;
	glm::vec2 m_nodePos;
};

class NodeFactory
{
public:
	NodeFactory();
	NodeFactory(NodeGraphPtr ownerGraph);

	NodeConstPtr getNodeDefinition() const { return m_nodeDefinition; }
	virtual NodePtr createNode(const class NodeEditorState& editorState) const;

protected:
	NodePtr m_nodeDefinition;
};