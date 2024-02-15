#pragma once

#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "RendererFwd.h"

#include <vector>

using SelectionComponentWeakList = std::vector<SelectionComponentWeakPtr>;

class MikanScene final
{
public:
	MikanScene();
	~MikanScene();

	void init();
	void dispose();

	const SelectionComponentWeakList& getSelectionComponentList() const { return m_selectionComponents; }

	void addMikanObject(MikanObjectPtr objectPtr);
	void removeMikanObject(MikanObjectConstPtr objectPtr);
	void addMikanComponent(MikanComponentPtr componentPtr);
	void removeMikanComponent(MikanComponentConstPtr componentPtr);

	void render(GlCameraConstPtr camera) const;

private:
	GlScenePtr m_glScene;
	SelectionComponentWeakList m_selectionComponents;
};