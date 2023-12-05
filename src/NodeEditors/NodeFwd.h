#pragma once

#include <memory>

typedef unsigned int        ImU32;  // 32-bit unsigned integer (often used to store packed colors)

typedef int t_node_id;
typedef int t_node_pin_id;
typedef int t_node_link_id;
typedef int t_graph_property_id;

class NodeEditorState;

class NodeFactory;
using NodeFactoryPtr = std::shared_ptr<NodeFactory>;
using NodeFactoryConstPtr = std::shared_ptr<const NodeFactory>;

class NodeLink;
using NodeLinkPtr = std::shared_ptr<NodeLink>;
using NodeLinkConstPtr = std::shared_ptr<const NodeLink>;

class NodeGraph;
using NodeGraphPtr = std::shared_ptr<NodeGraph>;
using NodeGraphConstPtr = std::shared_ptr<const NodeGraph>;

// Node Types
class Node;
using NodePtr = std::shared_ptr<Node>;
using NodeConstPtr = std::shared_ptr<const Node>;

// Pin Types
class NodePin;
using NodePinPtr = std::shared_ptr<NodePin>;
using NodePinConstPtr = std::shared_ptr<const NodePin>;

class TexturePin;
using TexturePinPtr = std::shared_ptr<TexturePin>;
using TexturePinConstPtr = std::shared_ptr<const TexturePin>;

// Graph Properties
class GraphProperty;
using GraphPropertyPtr = std::shared_ptr<GraphProperty>;
using GraphPropertyConstPtr = std::shared_ptr<const GraphProperty>;

template <typename t_element_type> 
class GraphArrayProperty;
template <typename t_element_type>
using GraphPropertyArrayPtr = std::shared_ptr<GraphArrayProperty<t_element_type>>;
template <typename t_element_type>
using GraphPropertyArrayConstPtr = std::shared_ptr<const GraphArrayProperty<t_element_type>>;

class GlFrameBuffer;
using GlFrameBufferPtr = std::shared_ptr<GlFrameBuffer>;
using FrameBufferArrayProperty = GraphArrayProperty<GlFrameBufferPtr>;
using FrameBufferArrayPropertyPtr = GraphPropertyArrayPtr<GlFrameBufferPtr>;
using FrameBufferArrayPropertyConstPtr = GraphPropertyArrayConstPtr<GlFrameBufferPtr>;;
