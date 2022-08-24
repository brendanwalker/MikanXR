#include "FrameTimer.h"
#include "SDL_timer.h"

FrameTimer::FrameTimer(int tickInterval)
	: m_tickInterval(tickInterval)
	, m_nextTime(SDL_GetTicks() + tickInterval)
{
}

void FrameTimer::waitForNextFrame()
{
	SDL_Delay(getTicksToNextFrame());

	m_nextTime += m_tickInterval;
}

Uint32 FrameTimer::getTicksToNextFrame() const
{
	Uint32 now = SDL_GetTicks();

	return (m_nextTime <= now) ? 0 : m_nextTime - now;
}