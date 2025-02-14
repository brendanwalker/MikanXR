#pragma once

#include "SDL_stdinc.h"

class FrameTimer
{
public:
	explicit FrameTimer(int tickInterval = 30);

	void waitForNextFrame();

private:
	const int m_tickInterval;
	Uint32 m_nextTime;

	Uint32 getTicksToNextFrame() const;
};