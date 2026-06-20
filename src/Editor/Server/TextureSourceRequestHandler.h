#pragma once

#include "IServerRequestHandler.h"

class TextureSourceRequestHandler : public IServerRequestHandler
{
public:
	TextureSourceRequestHandler(class MikanServer* owner)
		: IServerRequestHandler(owner)
	{
	}

	virtual bool startup(class MainWindow* mainWindow) override;

	// Video Source Events
	void publishTextureSourceOpenedEvent();
	void publishTextureSourceClosedEvent();
};