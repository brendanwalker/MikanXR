#include "EditorObjectSystem.h"

EditorObjectSystem::EditorObjectSystem() : MikanObjectSystem()
{
}

EditorObjectSystem::~EditorObjectSystem()
{
	dispose();
}

void EditorObjectSystem::init()
{
	MikanObjectSystem::init();
}

void EditorObjectSystem::dispose()
{
	m_selectionComponents.clear();
}

void EditorObjectSystem::registerSelectionComponent(SelectionComponentWeakPtr selectionComponentPtr)
{
	m_selectionComponents.push_back(selectionComponentPtr);
}

void EditorObjectSystem::unregisterSelectionComponent(SelectionComponentWeakPtr selectionComponentPtr)
{
	for (auto it = m_selectionComponents.begin(); it != m_selectionComponents.end(); it++)
	{
		if (it->lock() == selectionComponentPtr.lock())
		{
			m_selectionComponents.erase(it);
			return;
		}
	}
}
