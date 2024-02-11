#pragma once

#include "EditorNodeConstants.h"
#include "GlTypesFwd.h"

struct ImVec2;

namespace EditorNodeUtil
{
	EditorPinType GLTypeToPinType(GLenum type);
	int PinTypeSize(EditorPinType type);
	int GLDrawModeToIndex(GLenum drawMode);
	GLenum IndexToGLDrawMode(int index);
	ImVec2 MousePosToGridSpace();
};
