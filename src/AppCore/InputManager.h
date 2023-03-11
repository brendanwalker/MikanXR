#pragma once

#include "MulticastDelegate.h"

#include <map>
#include <vector>

#include <stdint.h>

typedef union SDL_Event SDL_Event;
typedef int32_t Sint32;
typedef Sint32 SDL_Keycode;

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

	std::map<SDL_Keycode, KeyEventBindings*> keybindings;
	MulticastDelegate<void(int dx, int dy)> OnMouseMotionEvent;
	MulticastDelegate<void(int button)> OnMouseButtonPressedEvent;
	MulticastDelegate<void(int button)> OnMouseButtonReleasedEvent;
	MulticastDelegate<void(int scroll)> OnMouseWheelScrolledEvent;

	void clear();
};

class InputManager
{
public:
	InputManager();
	virtual ~InputManager();

	static InputManager* getInstance() { return m_inputManager; }

	void onSDLEvent(SDL_Event& e);

	void getMouseScreenPosition(int &outScreenX, int &outScreenY) const;

	KeyEventBindings* getKeyBindings(SDL_Keycode key);
	KeyEventBindings* fetchOrAddKeyBindings(SDL_Keycode key);

	EventBindingSet* pushEventBindingSet();
	void popEventBindingSet();
	EventBindingSet* getCurrentEventBindingSet();

private:
	static InputManager* m_inputManager;
	std::vector<EventBindingSet*> m_eventBindings;
};
