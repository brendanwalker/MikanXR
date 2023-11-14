#pragma once

#include <memory>

class EditorNode;
using EditorNodePtr = std::shared_ptr<EditorNode>;
using EditorNodeConstPtr = std::shared_ptr<const EditorNode>;

class EditorPin;
using EditorPinPtr = std::shared_ptr<EditorPin>;
using EditorPinConstPtr = std::shared_ptr<const EditorPin>;

class EditorLink;
using EditorLinkPtr = std::shared_ptr<EditorLink>;
using EditorLinkConstPtr = std::shared_ptr<const EditorLink>;