#pragma once

#include "MikanObjectSystem.h"
#include "ComponentFwd.h"
#include "SceneFwd.h"

class EditorObjectSystem : public MikanObjectSystem
{
public:
	EditorObjectSystem();
	virtual ~EditorObjectSystem();

	virtual void init() override;
	virtual void dispose() override;

	void registerSelectionComponent(SelectionComponentWeakPtr selectionComponentPtr);
	void unregisterSelectionComponent(SelectionComponentWeakPtr selectionComponentPtr);

protected:
	std::vector<SelectionComponentWeakPtr> m_selectionComponents;
	MikanScenePtr m_scene;
};