#pragma once

class MkWindowEvent;

class IMkWindowEventListener
{
public:
	virtual bool onWindowEvent(const MkWindowEvent& event)= 0;
};
