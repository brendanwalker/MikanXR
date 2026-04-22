#include "SdlWindowEventListener.h"

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_events.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#endif

SdlWindowEvent::SdlWindowEvent(const SDL_Event* event)
	: m_event(event)
{
}

eMkWindowEventType SdlWindowEvent::getEventType() const
{
	switch (m_event->type)
	{
		case SDL_QUIT:
			return eMkWindowEventType::Quit;
		case SDL_KEYDOWN:
			return eMkWindowEventType::KeyDown;
		case SDL_KEYUP:
			return eMkWindowEventType::KeyUp;
		case SDL_MOUSEWHEEL:
			return eMkWindowEventType::MouseWheel;
		case SDL_MOUSEBUTTONDOWN:
			return eMkWindowEventType::MouseButtonDown;
		case SDL_MOUSEBUTTONUP:
			return eMkWindowEventType::MouseButtonUp;
		case SDL_MOUSEMOTION:
			return eMkWindowEventType::MouseMotion;
		case SDL_WINDOWEVENT:
			return eMkWindowEventType::WindowEvent;
		default:
			return eMkWindowEventType::Other;
	}
}

MkKeySym SdlWindowEvent::getKeySym() const
{
	if (m_event->type == SDL_KEYDOWN || m_event->type == SDL_KEYUP)
	{
		return m_event->key.keysym.sym;
	}

	return MkKey::UNKNOWN;
}

int SdlWindowEvent::getKeyRepeat() const
{
	if (m_event->type == SDL_KEYDOWN || m_event->type == SDL_KEYUP)
	{
		return m_event->key.repeat;
	}
	return 0;
}

eMkWindowEventID SdlWindowEvent::getWindowEventID() const
{
	if (m_event->type == SDL_WINDOWEVENT)
	{
		switch (m_event->window.event)
		{
			case SDL_WINDOWEVENT_SHOWN:       return eMkWindowEventID::Shown;
			case SDL_WINDOWEVENT_HIDDEN:      return eMkWindowEventID::Hidden;
			case SDL_WINDOWEVENT_SIZE_CHANGED:return eMkWindowEventID::SizeChanged;
			case SDL_WINDOWEVENT_ENTER:       return eMkWindowEventID::Enter;
			case SDL_WINDOWEVENT_LEAVE:       return eMkWindowEventID::Leave;
			case SDL_WINDOWEVENT_FOCUS_GAINED:return eMkWindowEventID::FocusGained;
			case SDL_WINDOWEVENT_FOCUS_LOST:  return eMkWindowEventID::FocusLost;
			case SDL_WINDOWEVENT_MINIMIZED:   return eMkWindowEventID::Minimized;
			case SDL_WINDOWEVENT_MAXIMIZED:   return eMkWindowEventID::Maximized;
			case SDL_WINDOWEVENT_RESTORED:    return eMkWindowEventID::Restored;
			case SDL_WINDOWEVENT_CLOSE:       return eMkWindowEventID::Close;
			default:                          return eMkWindowEventID::Unknown;
		}
	}
	return eMkWindowEventID::Unknown;
}

int SdlWindowEvent::getMouseWheelScrollAmount() const
{
	if (m_event->type == SDL_MOUSEWHEEL)
		return m_event->wheel.y;
	return 0;
}

int SdlWindowEvent::getMouseButton() const
{
	if (m_event->type == SDL_MOUSEBUTTONDOWN || m_event->type == SDL_MOUSEBUTTONUP)
		return m_event->button.button;
	return 0;
}

int SdlWindowEvent::getMouseMotionXRel() const
{
	if (m_event->type == SDL_MOUSEMOTION)
		return m_event->motion.xrel;
	return 0;
}

int SdlWindowEvent::getMouseMotionYRel() const
{
	if (m_event->type == SDL_MOUSEMOTION)
		return m_event->motion.yrel;
	return 0;
}

int SdlWindowEvent::getWindowData1() const
{
	if (m_event->type == SDL_WINDOWEVENT)
		return m_event->window.data1;
	return 0;
}

int SdlWindowEvent::getWindowData2() const
{
	if (m_event->type == SDL_WINDOWEVENT)
		return m_event->window.data2;
	return 0;
}

const void* SdlWindowEvent::getInternalWindowEvent() const
{
	return m_event;
}
