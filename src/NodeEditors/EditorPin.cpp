#include "EditorPin.h"

EditorLink::EditorLink() : id(-1)
{}

EditorPin::EditorPin()
	: id(-1)
	, type(EditorPinType::FLOW)
	, size(1)
	, isOutput(false)
{}

EditorFloatPin::EditorFloatPin()
	: EditorPin()
	, value(0.f)
{}

EditorFloat2Pin::EditorFloat2Pin()
	: EditorPin()
	, value{}
{
}

EditorFloat3Pin::EditorFloat3Pin()
	: EditorPin()
	, value{}
{}

EditorFloat4Pin::EditorFloat4Pin()
	: EditorPin()
	, value{}
{}

EditorIntPin::EditorIntPin()
	: EditorPin()
	, value(0.f)
{}

EditorInt2Pin::EditorInt2Pin()
	: EditorPin()
	, value{}
{}

EditorInt3Pin::EditorInt3Pin()
	: EditorPin()
	, value{}
{}

EditorInt4Pin::EditorInt4Pin()
	: EditorPin()
	, value{}
{}

EditorMat4Pin::EditorMat4Pin()
	: EditorPin()
	, value(glm::mat4(1.f))
{}

EditorBlockPin::EditorBlockPin()
	: blockPinType(EditorBlockPinType::UNIFROM_BLOCK)
	, index(-1)
{}