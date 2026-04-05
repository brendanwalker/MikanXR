#pragma once

typedef union SDL_Event SDL_Event;

class IMkWindowEventListener
{
public:
	virtual bool onWindowEvent(const SDL_Event* event) = 0;
};
