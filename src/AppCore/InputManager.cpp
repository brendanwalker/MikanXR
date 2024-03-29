#include "InputManager.h"

#if defined(_WIN32)
#include <SDL_events.h>
#include <SDL_keycode.h>
#else
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#endif

// -- InputManager ------
InputManager* InputManager::m_inputManager= nullptr;

InputManager::InputManager()
{
	m_inputManager= this;
}

InputManager::~InputManager()
{
	while (m_eventBindings.size() > 0)
	{
		popEventBindingSet();
	}

	m_inputManager= nullptr;
}

bool InputManager::onSDLEvent(const SDL_Event* event)
{	
	bool bHandled= false;

	switch (event->type)
	{
	case SDL_KEYDOWN:
		{
			KeyEventBindings* keybinds= getKeyBindings(event->key.keysym.sym);
			if (keybinds != nullptr && keybinds->OnKeyPressed)
			{
				keybinds->OnKeyPressed();
				bHandled = true;
			}
		} break;
	case SDL_KEYUP:
		{
			KeyEventBindings* keybinds = getKeyBindings(event->key.keysym.sym);
			if (keybinds != nullptr)
			{
				if (event->key.repeat > 0)
				{
					if (keybinds->OnKeyRepeated)
					{
						keybinds->OnKeyRepeated();
						bHandled= true;
					}
				}
				else
				{
					if (keybinds->OnKeyReleased)
					{
						keybinds->OnKeyReleased();
						bHandled = true;
					}
				}
			}
		} break;
	case SDL_MOUSEWHEEL:
		{
			EventBindingSet* bindingSet = getCurrentEventBindingSet();
			if (bindingSet != nullptr && bindingSet->OnMouseWheelScrolledEvent)
			{
				bindingSet->OnMouseWheelScrolledEvent(event->wheel.y);
				bHandled = true;
			}
		} break;
	case SDL_MOUSEBUTTONDOWN:
		{
			EventBindingSet* bindingSet = getCurrentEventBindingSet();
			if (bindingSet != nullptr && bindingSet->OnMouseButtonPressedEvent)
			{
				bindingSet->OnMouseButtonPressedEvent(event->button.button);
				bHandled = true;
			}
		} break;
	case SDL_MOUSEBUTTONUP:
		{
			EventBindingSet* bindingSet = getCurrentEventBindingSet();
			if (bindingSet != nullptr && bindingSet->OnMouseButtonReleasedEvent)
			{
				bindingSet->OnMouseButtonReleasedEvent(event->button.button);
				bHandled = true;
			}
		} break;
	case SDL_MOUSEMOTION:
		{
			EventBindingSet* bindingSet = getCurrentEventBindingSet();
			if (bindingSet != nullptr && bindingSet->OnMouseMotionEvent)
			{
				bindingSet->OnMouseMotionEvent(event->motion.xrel, event->motion.yrel);
				bHandled = true;
			}
		} break;
	default:
		break;
	}

	return bHandled;
}

void InputManager::getMouseScreenPosition(int& outScreenX, int& outScreenY) const
{
	SDL_GetMouseState(&outScreenX, &outScreenY);
}

KeyEventBindings* InputManager::getKeyBindings(SDL_Keycode key)
{
	EventBindingSet* bindingSet= getCurrentEventBindingSet();
	if (bindingSet == nullptr)
		return nullptr;

	auto it= bindingSet->keybindings.find(key);
	if (it != bindingSet->keybindings.end())
	{
		return it->second;
	}

	return nullptr;
}

KeyEventBindings* InputManager::fetchOrAddKeyBindings(SDL_Keycode key)
{
	EventBindingSet* bindingSet = getCurrentEventBindingSet();
	if (bindingSet == nullptr)
	{
		bindingSet = pushEventBindingSet();
	}

	auto it = bindingSet->keybindings.find(key);
	if (it != bindingSet->keybindings.end())
	{
		return it->second;
	}
	else
	{
		KeyEventBindings* emptyBindings = new KeyEventBindings();

		bindingSet->keybindings.insert({ key, emptyBindings });

		return emptyBindings;
	}
}

EventBindingSet* InputManager::pushEventBindingSet()
{
	EventBindingSet* newEventBindingSet = new EventBindingSet();

	m_eventBindings.push_back(newEventBindingSet);

	return newEventBindingSet;
}

void InputManager::popEventBindingSet()
{
	if (m_eventBindings.size() > 0)
	{
		delete m_eventBindings[m_eventBindings.size() - 1];
		m_eventBindings.pop_back();
	}
}

EventBindingSet* InputManager::getCurrentEventBindingSet()
{
	if (m_eventBindings.size() > 0)
	{
		return m_eventBindings[m_eventBindings.size() - 1];
	}
	else
	{
		return nullptr;
	}
}

// -- KeyEventBindings -----
KeyEventBindings::~KeyEventBindings()
{
	clear();
}

void KeyEventBindings::clear()
{
	OnKeyPressed.Clear();
	OnKeyReleased.Clear();
	OnKeyRepeated.Clear();
}

// -- EventBindingSet -----
EventBindingSet::~EventBindingSet()
{
	clear();
}

void EventBindingSet::clear()
{
	for (auto it = keybindings.begin(); it != keybindings.end(); ++it)
	{
		delete it->second;
	}
	keybindings.clear();

	OnMouseMotionEvent.Clear();
	OnMouseButtonPressedEvent.Clear();
	OnMouseButtonReleasedEvent.Clear();
	OnMouseWheelScrolledEvent.Clear();
}
