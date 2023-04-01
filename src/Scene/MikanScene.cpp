#include "MikanScene.h"
#include "MikanObject.h"
#include "IGLSceneRenderable.h"
#include "GlScene.h"

MikanScene::MikanScene()
	: m_glScene(new GlScene())
{

}

MikanScene::~MikanScene()
{
	dispose();
	delete m_glScene;
}

void MikanScene::init()
{
}

void MikanScene::dispose()
{
	m_objects.clear();
}

void MikanScene::addMikanObject(MikanObjectPtr objectPtr)
{
	m_objects.push_back(objectPtr);

	std::vector<IGlSceneRenderable*> renderableComponents;
	objectPtr->getComponentsOfType(renderableComponents);
	for (IGlSceneRenderable* renderableComponent : renderableComponents)
	{
		m_glScene->addInstance(renderableComponent);
	}	
}

void MikanScene::removeMikanObject(MikanObjectPtr objectPtr)
{
	auto it= std::find(m_objects.begin(), m_objects.end(), objectPtr);
	if (it != m_objects.end())
	{
		MikanObjectPtr objectPtr= *it;

		std::vector<IGlSceneRenderable*> renderableComponents;
		objectPtr->getComponentsOfType(renderableComponents);
		for (IGlSceneRenderable* renderableComponent : renderableComponents)
		{
			m_glScene->removeInstance(renderableComponent);
		}

		m_objects.erase(it);
	}
}

void MikanScene::update()
{
	for (MikanObjectPtr objectPtr : m_objects)
	{
		objectPtr->update();
	}
}

void MikanScene::render()
{
	m_glScene->render();
}