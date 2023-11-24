#pragma once

#include <memory>

class NodeEditorWindow;
using NodeEditorWindowPtr = std::shared_ptr<NodeEditorWindow>;

class EditorNode;
using EditorNodePtr = std::shared_ptr<EditorNode>;
using EditorNodeConstPtr = std::shared_ptr<const EditorNode>;

class EditorBlockNode;
using EditorBlockNodePtr = std::shared_ptr<EditorBlockNode>;
using EditorBlockNodeConstPtr = std::shared_ptr<const EditorBlockNode>;

class EditorEventNode;
using EditorEventNodePtr = std::shared_ptr<EditorEventNode>;
using EditorEventNodeConstPtr = std::shared_ptr<const EditorEventNode>;

class EditorImageNode;
using EditorImageNodePtr = std::shared_ptr<EditorImageNode>;
using EditorImageNodeConstPtr = std::shared_ptr<const EditorImageNode>;

class EditorPingPongNode;
using EditorPingPongNodePtr = std::shared_ptr<EditorPingPongNode>;
using EditorPingPongNodeConstPtr = std::shared_ptr<const EditorPingPongNode>;

class EditorProgramNode;
using EditorProgramNodePtr = std::shared_ptr<EditorProgramNode>;
using EditorProgramNodeConstPtr = std::shared_ptr<const EditorProgramNode>;

class EditorTextureNode;
using EditorTextureNodePtr = std::shared_ptr<EditorTextureNode>;
using EditorTextureNodeConstPtr = std::shared_ptr<const EditorTextureNode>;

class EditorPin;
using EditorPinPtr = std::shared_ptr<EditorPin>;
using EditorPinConstPtr = std::shared_ptr<const EditorPin>;

class EditorBlockPin;
using EditorBlockPinPtr = std::shared_ptr<EditorBlockPin>;
using EditorBlockPinConstPtr = std::shared_ptr<const EditorBlockPin>;

class EditorIntPin;
using EditorIntPinPtr = std::shared_ptr<EditorIntPin>;
using EditorIntPinConstPtr = std::shared_ptr<const EditorIntPin>;

class EditorInt2Pin;
using EditorInt2PinPtr = std::shared_ptr<EditorInt2Pin>;
using EditorInt2PinConstPtr = std::shared_ptr<const EditorInt2Pin>;

class EditorInt3Pin;
using EditorInt3PinPtr = std::shared_ptr<EditorInt3Pin>;
using EditorInt3PinConstPtr = std::shared_ptr<const EditorInt3Pin>;

class EditorInt4Pin;
using EditorInt4PinPtr = std::shared_ptr<EditorInt4Pin>;
using EditorInt4PinConstPtr = std::shared_ptr<const EditorInt4Pin>;

class EditorFloatPin;
using EditorFloatPinPtr = std::shared_ptr<EditorFloatPin>;
using EditorFloatPinConstPtr = std::shared_ptr<const EditorFloatPin>;

class EditorFloat2Pin;
using EditorFloat2PinPtr = std::shared_ptr<EditorFloat2Pin>;
using EditorFloat2PinConstPtr = std::shared_ptr<const EditorFloat2Pin>;

class EditorFloat3Pin;
using EditorFloat3PinPtr = std::shared_ptr<EditorFloat3Pin>;
using EditorFloat3PinConstPtr = std::shared_ptr<const EditorFloat3Pin>;

class EditorFloat4Pin;
using EditorFloat4PinPtr = std::shared_ptr<EditorFloat4Pin>;
using EditorFloat4PinConstPtr = std::shared_ptr<const EditorFloat4Pin>;

class EditorLink;
using EditorLinkPtr = std::shared_ptr<EditorLink>;
using EditorLinkConstPtr = std::shared_ptr<const EditorLink>;

class NodeGraph;
using NodeGraphPtr = std::shared_ptr<NodeGraph>;
using NodeGraphConstPtr = std::shared_ptr<const NodeGraph>;