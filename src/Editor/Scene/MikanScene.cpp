#include "MikanScene.h"
#include "MikanObject.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "IMkSceneRenderable.h"
#include "MikanCamera.h"
#include "GlScene.h"

MikanScene::MikanScene()
	: m_mkScene(std::make_shared<GlScene>())
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
	m_mkScene= nullptr;
	m_selectionComponents.clear();
}

void MikanScene::addMikanObject(MikanObjectPtr objectPtr)
{
	if (!objectPtr)
		return;

	for (MikanComponentPtr componentPtr : objectPtr->getComponentsConst())
	{
		addMikanComponent(componentPtr);
	}
}

void MikanScene::removeMikanObject(MikanObjectConstPtr objectPtr)
{
	if (!objectPtr)
		return;

	for (MikanComponentPtr componentPtr : objectPtr->getComponentsConst())
	{
		removeMikanComponent(componentPtr);
	}
}

void MikanScene::addMikanComponent(MikanComponentPtr componentPtr)
{
	if (!componentPtr)
		return;

	// See if the given component is a scene component
	SceneComponentPtr sceneComponent= std::dynamic_pointer_cast<SceneComponent>(componentPtr);
	if (sceneComponent != nullptr)
	{
		IMkSceneRenderableConstPtr renderable = sceneComponent->getGlSceneRenderableConst();
		if (renderable)
		{
			// If the scene component has a renderable, add it to the GL Scene
			m_mkScene->addInstance(renderable);
		}
	}
	// See if the given component is a selection component
	SelectionComponentPtr selectionComponent = std::dynamic_pointer_cast<SelectionComponent>(componentPtr);
	if (selectionComponent != nullptr)
	{
		bool bHasSelectionComponent= false;
		for (auto component_it = m_selectionComponents.begin(); component_it != m_selectionComponents.end(); ++component_it)
		{
			if (selectionComponent == component_it->lock())
			{
				bHasSelectionComponent= true;
				break;
			}
		}

		if (!bHasSelectionComponent)
		{
			m_selectionComponents.push_back(selectionComponent);
		}
	}
}

void MikanScene::removeMikanComponent(MikanComponentConstPtr componentPtr)
{
	if (!componentPtr)
		return;

	// See if the given component is a scene component
	SceneComponentConstPtr sceneComponent = std::dynamic_pointer_cast<const SceneComponent>(componentPtr);
	if (sceneComponent != nullptr)
	{
		IMkSceneRenderableConstPtr renderable = sceneComponent->getGlSceneRenderableConst();
		if (renderable)
		{
			// If the scene component has a renderable, remove it from the GL Scene
			m_mkScene->removeInstance(renderable);
		}
	}
	// See if the given component is a selection component
	SelectionComponentConstPtr selectionComponent = std::dynamic_pointer_cast<const SelectionComponent>(componentPtr);
	if (selectionComponent != nullptr)
	{
		for (auto component_it = m_selectionComponents.begin(); component_it != m_selectionComponents.end(); ++component_it)
		{
			if (selectionComponent == component_it->lock())
			{
				m_selectionComponents.erase(component_it);
				break;
			}
		}
	}
}

void MikanScene::render(MikanCameraConstPtr camera, MkStateStack& MkStateStack) const
{
	m_mkScene->render(camera, MkStateStack);
}