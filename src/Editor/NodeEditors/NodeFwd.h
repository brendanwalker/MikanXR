#pragma once

#include <memory>

typedef unsigned int        ImU32;  // 32-bit unsigned integer (often used to store packed colors)

typedef int t_node_id;
typedef int t_node_pin_id;
typedef int t_node_link_id;
typedef int t_graph_property_id;

// Node Editor
class NodeEditorState;
class NodeEvaluator;

// Node Link
class NodeLink;
using NodeLinkPtr = std::shared_ptr<NodeLink>;
using NodeLinkConstPtr = std::shared_ptr<const NodeLink>;

class NodeLinkConfig;
using NodeLinkConfigPtr = std::shared_ptr<NodeLinkConfig>;
using NodeLinkConfigConstPtr = std::shared_ptr<const NodeLinkConfig>;

// Node Graph
class NodeGraph;
using NodeGraphPtr = std::shared_ptr<NodeGraph>;
using NodeGraphConstPtr = std::shared_ptr<const NodeGraph>;

class NodeGraphConfig;
using NodeGraphConfigPtr = std::shared_ptr<NodeGraphConfig>;
using NodeGraphConfigConstPtr = std::shared_ptr<const NodeGraphConfig>;

// Compositor Node Graph
class CompositorNodeGraph;
using CompositorNodeGraphPtr = std::shared_ptr<CompositorNodeGraph>;
using CompositorNodeGraphConstPtr = std::shared_ptr<const CompositorNodeGraph>;

// Node Types
class NodeFactory;
using NodeFactoryPtr = std::shared_ptr<NodeFactory>;
using NodeFactoryConstPtr = std::shared_ptr<const NodeFactory>;

class Node;
using NodePtr = std::shared_ptr<Node>;
using NodeConstPtr = std::shared_ptr<const Node>;

class NodeConfig;
using NodeConfigPtr = std::shared_ptr<NodeConfig>;
using NodeConfigConstPtr = std::shared_ptr<const NodeConfig>;

class NodeGraphFactory;
using NodeGraphFactoryPtr = std::shared_ptr<NodeGraphFactory>;
using NodeGraphFactoryConstPtr = std::shared_ptr<const NodeGraphFactory>;

class EventNode;
using EventNodePtr = std::shared_ptr<EventNode>;
using EventNodeConstPtr = std::shared_ptr<const EventNode>;

class MousePosNode;
using MousePosNodePtr = std::shared_ptr<MousePosNode>;
using MousePosNodeConstPtr = std::shared_ptr<const MousePosNode>;

class DepthMaskNode;
using DepthMaskNodePtr = std::shared_ptr<DepthMaskNode>;
using DepthMaskNodeConstPtr = std::shared_ptr<const DepthMaskNode>;

class DrawLayerNode;
using DrawLayerNodePtr = std::shared_ptr<DrawLayerNode>;
using DrawLayerNodeConstPtr = std::shared_ptr<const DrawLayerNode>;

class MaterialNode;
using MaterialNodePtr = std::shared_ptr<MaterialNode>;
using MaterialNodeConstPtr = std::shared_ptr<const MaterialNode>;

class ModelNode;
using ModelNodePtr = std::shared_ptr<ModelNode>;
using ModelNodeConstPtr = std::shared_ptr<const ModelNode>;

class StencilNode;
using StencilNodePtr = std::shared_ptr<StencilNode>;
using StencilNodeConstPtr = std::shared_ptr<const StencilNode>;

class TextureNode;
using TextureNodePtr = std::shared_ptr<TextureNode>;
using TextureNodeConstPtr = std::shared_ptr<const TextureNode>;

class TimeNode;
using TimeNodePtr = std::shared_ptr<TimeNode>;
using TimeNodeConstPtr = std::shared_ptr<const TimeNode>;

class VideoTextureNode;
using VideoTextureNodePtr = std::shared_ptr<VideoTextureNode>;
using VideoTextureNodeConstPtr = std::shared_ptr<const VideoTextureNode>;

class VariableNode;
using VariableNodePtr = std::shared_ptr<VariableNode>;
using VariableNodeConstPtr = std::shared_ptr<const VariableNode>;

// Pin Types
class ArrayPin;
using ArrayPinPtr = std::shared_ptr<ArrayPin>;
using ArrayPinConstPtr = std::shared_ptr<const ArrayPin>;

class BoolPin;
using BoolPinPtr = std::shared_ptr<BoolPin>;
using BoolPinConstPtr = std::shared_ptr<const BoolPin>;

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

class NodePinConfig;
using NodePinConfigPtr = std::shared_ptr<NodePinConfig>;
using NodePinConfigConstPtr = std::shared_ptr<const NodePinConfig>;

class NodePinFactory;
using NodePinFactoryPtr = std::shared_ptr<NodePinFactory>;
using NodePinFactoryConstPtr = std::shared_ptr<const NodePinFactory>;

class PropertyPin;
using PropertyPinPtr = std::shared_ptr<PropertyPin>;
using PropertyPinConstPtr = std::shared_ptr<const PropertyPin>;

class TexturePin;
using TexturePinPtr = std::shared_ptr<TexturePin>;
using TexturePinConstPtr = std::shared_ptr<const TexturePin>;

class ValuePin;
using ValuePinPtr = std::shared_ptr<ValuePin>;
using ValuePinConstPtr = std::shared_ptr<const ValuePin>;

// Graph Properties
class GraphProperty;
using GraphPropertyPtr = std::shared_ptr<GraphProperty>;
using GraphPropertyConstPtr = std::shared_ptr<const GraphProperty>;

class GraphPropertyConfig;
using GraphPropertyConfigPtr = std::shared_ptr<GraphPropertyConfig>;
using GraphPropertyConfigConstPtr = std::shared_ptr<const GraphPropertyConfig>;

class GraphPropertyFactory;
using GraphPropertyFactoryPtr = std::shared_ptr<GraphPropertyFactory>;
using GraphPropertyFactoryConstPtr = std::shared_ptr<const GraphPropertyFactory>;

class GraphArrayProperty;
using GraphArrayPropertyPtr = std::shared_ptr<GraphArrayProperty>;
using GraphArrayPropertyConstPtr = std::shared_ptr<const GraphArrayProperty>;

class GraphMaterialProperty;
using GraphMaterialPropertyPtr = std::shared_ptr<GraphMaterialProperty>;
using GraphMaterialPropertyConstPtr = std::shared_ptr<const GraphMaterialProperty>;

class GraphModelProperty;
using GraphModelPropertyPtr = std::shared_ptr<GraphModelProperty>;
using GraphModelPropertyConstPtr = std::shared_ptr<const GraphModelProperty>;

class GraphStencilProperty;
using GraphStencilPropertyPtr = std::shared_ptr<GraphStencilProperty>;
using GraphStencilPropertyConstPtr = std::shared_ptr<const GraphStencilProperty>;

class GraphTextureProperty;
using GraphTexturePropertyPtr = std::shared_ptr<GraphTextureProperty>;
using GraphTexturePropertyConstPtr = std::shared_ptr<const GraphTextureProperty>;

class GraphValueProperty;
using GraphValuePropertyPtr = std::shared_ptr<GraphValueProperty>;
using GraphValuePropertyConstPtr = std::shared_ptr<const GraphValueProperty>;