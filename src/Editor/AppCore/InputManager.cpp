#include "IEditorWindow.h"
#include "InputManager.h"
#include "MkWindowEvent.h"

// -- InputManager ------
InputManager::InputManager(class IEditorWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
}

InputManager::~InputManager()
{
	while (m_eventBindings.size() > 0)
	{
		popEventBindingSet();
	}
}

bool InputManager::onWindowEvent(const MkWindowEvent& event)
{	
	bool bHandled= false;

	switch (event.getEventType())
	{
	case eMkWindowEventType::KeyDown:
		{
			KeyEventBindings* keybinds= getKeyBindings(event.getKeySym());
			if (keybinds != nullptr && keybinds->OnKeyPressed)
			{
				keybinds->OnKeyPressed();
				bHandled = true;
			}
		} break;
	case eMkWindowEventType::KeyUp:
		{
			KeyEventBindings* keybinds = getKeyBindings(event.getKeySym());
			if (keybinds != nullptr)
			{
				if (event.getKeyRepeat())
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
	case eMkWindowEventType::MouseWheel:
		{
			EventBindingSet* bindingSet = getCurrentEventBindingSet();
			if (bindingSet != nullptr && bindingSet->OnMouseWheelScrolledEvent)
			{
				bindingSet->OnMouseWheelScrolledEvent(event.getMouseWheelScrollAmount());
				bHandled = true;
			}
		} break;
	case eMkWindowEventType::MouseButtonDown:
		{
			EventBindingSet* bindingSet = getCurrentEventBindingSet();
			if (bindingSet != nullptr && bindingSet->OnMouseButtonPressedEvent)
			{
				bindingSet->OnMouseButtonPressedEvent(event.getMouseButton());
				bHandled = true;
			}
		} break;
	case eMkWindowEventType::MouseButtonUp:
		{
			EventBindingSet* bindingSet = getCurrentEventBindingSet();
			if (bindingSet != nullptr && bindingSet->OnMouseButtonReleasedEvent)
			{
				bindingSet->OnMouseButtonReleasedEvent(event.getMouseButton());
				bHandled = true;
			}
		} break;
	case eMkWindowEventType::MouseMotion:
		{
			EventBindingSet* bindingSet = getCurrentEventBindingSet();
			if (bindingSet != nullptr && bindingSet->OnMouseMotionEvent)
			{
				bindingSet->OnMouseMotionEvent(event.getMouseMotionXRel(), event.getMouseMotionYRel());
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
	if (m_ownerWindow)
	{
		m_ownerWindow->getMouseScreenPosition(outScreenX, outScreenY);
	}
	else
	{
		outScreenX = 0;
		outScreenY = 0;
	}
}

KeyEventBindings* InputManager::getKeyBindings(MkKeySym key)
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

KeyEventBindings* InputManager::fetchOrAddKeyBindings(MkKeySym key)
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
