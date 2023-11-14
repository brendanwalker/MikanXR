#pragma once

#include "EditorNodeConstants.h"
#include "GlTypesFwd.h"

namespace EditorNodeUtil
{
	EditorPinType GLTypeToPinType(GLenum type);
	int PinTypeSize(EditorPinType type);
	int GLDrawModeToIndex(GLenum drawMode);
	GLenum IndexToGLDrawMode(int index);
};
