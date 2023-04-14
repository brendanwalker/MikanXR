#pragma once

#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "RendererFwd.h"

#include <vector>

using MikanObjectWeakList = std::vector<MikanObjectWeakPtr>;
using SelectionComponentWeakList = std::vector<SelectionComponentWeakPtr>;

class MikanScene final
{
public:
	MikanScene();
	~MikanScene();

	void init();
	void dispose();

	const MikanObjectWeakList& getObjectList() const { return m_objects; }
	const SelectionComponentWeakList& getSelectionComponentList() const { return m_selectionComponents; }

	void addMikanObject(MikanObjectWeakPtr objectWeakPtr);
	void removeMikanObject(MikanObjectWeakPtr objectWeakPtr);

	void update();
	void render();

private:
	GlScenePtr m_glScene;
	MikanObjectWeakList m_objects;
	SelectionComponentWeakList m_selectionComponents;
};