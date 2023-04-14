#include "EditorObjectSystem.h"
#include "MikanScene.h"

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

	m_scene = std::make_shared<MikanScene>();
	m_scene->init();
}

void EditorObjectSystem::dispose()
{
	m_scene= nullptr;
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
