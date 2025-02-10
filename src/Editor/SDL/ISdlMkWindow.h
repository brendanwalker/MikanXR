#pragma once

#include "IMkWindow.h"
#include "SdlFwd.h"

class ISdlMkWindow : public IMkWindow
{
public:
	virtual SdlWindow& getSdlWindow() = 0;
	virtual class MikanModelResourceManager* getModelResourceManager() = 0;

	virtual bool onSDLEvent(const SDL_Event* event)= 0;
};