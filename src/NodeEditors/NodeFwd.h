#pragma once

#include <memory>

typedef unsigned int        ImU32;  // 32-bit unsigned integer (often used to store packed colors)

typedef int t_node_id;
typedef int t_node_pin_id;
typedef int t_node_link_id;
typedef int t_graph_property_id;

class NodeEditorState;
class NodeEvaluator;

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

class EventNode;
using EventNodePtr = std::shared_ptr<EventNode>;
using EventNodeConstPtr = std::shared_ptr<const EventNode>;

class TimeNode;
using TimeNodePtr = std::shared_ptr<TimeNode>;
using TimeNodeConstPtr = std::shared_ptr<const TimeNode>;

// Pin Types
class FloatPin;
using FloatPinPtr = std::shared_ptr<FloatPin>;
using FloatPinConstPtr = std::shared_ptr<const FloatPin>;

class Float2Pin;
using Float2PinPtr = std::shared_ptr<Float2Pin>;
using Float2PinConstPtr = std::shared_ptr<const Float2Pin>;

class Float3Pin;
using Float3PinPtr = std::shared_ptr<Float3Pin>;
using Float3PinConstPtr = std::shared_ptr<const Float3Pin>;

class Float4Pin;
using Float4PinPtr = std::shared_ptr<Float4Pin>;
using Float4PinConstPtr = std::shared_ptr<const Float4Pin>;

class FlowPin;
using FlowPinPtr = std::shared_ptr<FlowPin>;
using FlowPinConstPtr = std::shared_ptr<const FlowPin>;

class IntPin;
using IntPinPtr = std::shared_ptr<IntPin>;
using IntPinConstPtr = std::shared_ptr<const IntPin>;

class Int2Pin;
using Int2PinPtr = std::shared_ptr<Int2Pin>;
using Int2PinConstPtr = std::shared_ptr<const Int2Pin>;

class Int3Pin;
using Int3PinPtr = std::shared_ptr<Int3Pin>;
using Int3PinConstPtr = std::shared_ptr<const Int3Pin>;

class Int4Pin;
using Int4PinPtr = std::shared_ptr<Int4Pin>;
using Int4PinConstPtr = std::shared_ptr<const Int4Pin>;

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
