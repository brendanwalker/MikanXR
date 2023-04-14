#pragma once

#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "RendererFwd.h"

#include <vector>

class MikanScene final
{
public:
	MikanScene();
	~MikanScene();

	void init();
	void dispose();

	void addMikanObject(MikanObjectPtr objectPtr);
	void removeMikanObject(MikanObjectPtr objectPtr);

	void update();
	void render();

private:
	GlScenePtr m_glScene;
	std::vector<MikanObjectPtr> m_objects;
	std::vector<ColliderComponentWeakPtr> m_colliders;
};