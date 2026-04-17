#pragma once

#include "MkWindowEvent.h"

typedef union SDL_Event SDL_Event;

class SdlWindowEvent : public MkWindowEvent
{
public:
	SdlWindowEvent(const SDL_Event* event);

	virtual eMkWindowEventType getEventType() const override;
	virtual MkKeySym getKeySym() const override;
	virtual int getKeyRepeat() const override;
	virtual eMkWindowEventID getWindowEventID() const override;
	virtual int getMouseWheelScrollAmount() const override;
	virtual int getMouseButton() const override;
	virtual int getMouseMotionXRel() const override;
	virtual int getMouseMotionYRel() const override;
	virtual int getWindowData1() const override;
	virtual int getWindowData2() const override;
	virtual const void* getInternalWindowEvent() const override;

private:
	const SDL_Event* m_event;
};
