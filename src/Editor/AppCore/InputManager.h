#pragma once

#include "MulticastDelegate.h"
#include "MkWindowEvent.h"

#include <map>
#include <vector>

#include <stdint.h>

class KeyEventBindings
{
public:
	KeyEventBindings() = default;
	virtual ~KeyEventBindings();

	MulticastDelegate<void()> OnKeyPressed;
	MulticastDelegate<void()> OnKeyReleased;
	MulticastDelegate<void()> OnKeyRepeated;

	void clear();
};

class EventBindingSet
{
public:
	EventBindingSet() = default;
	virtual ~EventBindingSet();

	std::map<MkKeySym, KeyEventBindings*> keybindings;
	MulticastDelegate<void(int dx, int dy)> OnMouseMotionEvent;
	MulticastDelegate<void(int button)> OnMouseButtonPressedEvent;
	MulticastDelegate<void(int button)> OnMouseButtonReleasedEvent;
	MulticastDelegate<void(int scroll)> OnMouseWheelScrolledEvent;

	void clear();
};

class InputManager
{
public:
	InputManager()= delete;
	InputManager(class IEditorWindow* ownerWindow);
	virtual ~InputManager();

	bool onWindowEvent(const MkWindowEvent& event);
	void getMouseScreenPosition(int &outScreenX, int &outScreenY) const;

	KeyEventBindings* getKeyBindings(MkKeySym key);
	KeyEventBindings* fetchOrAddKeyBindings(MkKeySym key);

	EventBindingSet* pushEventBindingSet();
	void popEventBindingSet();
	EventBindingSet* getCurrentEventBindingSet();

private:
	class IEditorWindow* m_ownerWindow;
	std::vector<EventBindingSet*> m_eventBindings;
};
