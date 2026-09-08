#pragma once

#include "MkWindowEvent.h"

#include <stdint.h>

// -------------------------------------------------------------------------------------------------
// CEFBrowserInputState
//
// Modifier state accompanying each input event forwarded into a windowless browser. Expressed in
// plain types and MikanWindow enums, and kept in its own header, so CEFBrowserEditorWindow can
// drive the browser without including any CEF header.
// -------------------------------------------------------------------------------------------------
struct CEFBrowserInputState
{
	uint16_t keyMod= MkKeyMod::NONE; // MkKeyMod flags held during the event
	bool bLeftButtonDown= false;
	bool bMiddleButtonDown= false;
	bool bRightButtonDown= false;
};
