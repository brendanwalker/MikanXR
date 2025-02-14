#pragma once

#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "MikanRendererFwd.h"

#include <vector>

using SelectionComponentWeakList = std::vector<SelectionComponentWeakPtr>;

class MikanScene final
{
public:
	MikanScene();
	~MikanScene();

	inline IMkScenePtr getMkScene() const { return m_mkScene; }

	void init();
	void dispose();

	const SelectionComponentWeakList& getSelectionComponentList() const { return m_selectionComponents; }

	void addMikanObject(MikanObjectPtr objectPtr);
	void removeMikanObject(MikanObjectConstPtr objectPtr);
	void addMikanComponent(MikanComponentPtr componentPtr);
	void removeMikanComponent(MikanComponentConstPtr componentPtr);

	void render(MikanCameraConstPtr camera, class MkStateStack& MkStateStack) const;

private:
	IMkScenePtr m_mkScene;
	SelectionComponentWeakList m_selectionComponents;
};