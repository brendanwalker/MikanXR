#include "MikanScene.h"
#include "MikanObject.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "IGLSceneRenderable.h"
#include "GlScene.h"

MikanScene::MikanScene()
	: m_glScene(std::make_shared<GlScene>())
{

}

MikanScene::~MikanScene()
{
	dispose();
}

void MikanScene::init()
{
}

void MikanScene::dispose()
{
	m_glScene= nullptr;
	m_objects.clear();
	m_selectionComponents.clear();
}

void MikanScene::addMikanObject(MikanObjectWeakPtr objectWeakPtr)
{
	MikanObjectPtr objectPtr= objectWeakPtr.lock();
	if (!objectPtr)
		return;

	m_objects.push_back(objectWeakPtr);

	// Add renderable components to the GlScene
	std::vector<SceneComponentPtr> sceneComponents;
	objectPtr->getComponentsOfType(sceneComponents);
	for (SceneComponentPtr sceneComponent : sceneComponents)
	{
		IGlSceneRenderableConstPtr renderable= sceneComponent->getGlSceneRenderableConst();
		
		if (renderable)
		{
			m_glScene->addInstance(renderable);
		}
	}

	// Track selection component for raycasts
	std::vector<SelectionComponentPtr> SelectionComponents;
	objectPtr->getComponentsOfType(SelectionComponents);
	for (SelectionComponentPtr SelectionComponent : SelectionComponents)
	{
		SelectionComponentWeakPtr weakPtr(SelectionComponent);
		m_selectionComponents.push_back(weakPtr);
	}
}

void MikanScene::removeMikanObject(MikanObjectWeakPtr objectWeakPtr)
{
	MikanObjectPtr objectPtr = objectWeakPtr.lock();
	if (!objectPtr)
		return;

	for (auto it = m_objects.begin(); it != m_objects.end(); it++)
	{
		// See if this the element we are looking for
		MikanObjectPtr elemPtr = it->lock();
		if (elemPtr != objectPtr)
			continue;

		// Remove renderable components from the GlScene
		std::vector<SceneComponentPtr> sceneComponents;
		elemPtr->getComponentsOfType(sceneComponents);
		for (SceneComponentPtr sceneComponent : sceneComponents)
		{
			IGlSceneRenderableConstPtr renderable= sceneComponent->getGlSceneRenderableConst();

			if (renderable)
			{
				m_glScene->removeInstance(renderable);
			}
		}

		// Forget about selection components associated with the object
		std::vector<SelectionComponentPtr> SelectionComponents;
		elemPtr->getComponentsOfType(SelectionComponents);
		for (SelectionComponentPtr SelectionComponent : SelectionComponents)
		{
			for (auto component_it = m_selectionComponents.begin(); component_it != m_selectionComponents.end(); ++component_it)
			{
				if (SelectionComponent == component_it->lock())
				{
					m_selectionComponents.erase(component_it);
					break;
				}
			}
		}

		m_objects.erase(it);
	}
}

void MikanScene::render()
{
	m_glScene->render();
}