#include "MikanScene.h"
#include "MikanObject.h"
#include "MikanSceneComponent.h"
#include "MikanColliderComponent.h"
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

	// Add renderable components to the GlScene
	std::vector<MikanSceneComponentPtr> sceneComponents;
	objectPtr->getComponentsOfType(sceneComponents);
	for (MikanSceneComponentPtr sceneComponent : sceneComponents)
	{
		IGlSceneRenderableConstPtr renderable= sceneComponent->getGlSceneRenderableConst();
		
		if (renderable)
		{
			m_glScene->addInstance(renderable);
		}
	}

	// Track collidable component for raycasts
	std::vector<MikanColliderComponentPtr> colliderComponents;
	objectPtr->getComponentsOfType(colliderComponents);
	for (MikanColliderComponentPtr colliderComponent : colliderComponents)
	{
		MikanColliderComponentWeakPtr weakPtr(colliderComponent);
		m_colliders.push_back(weakPtr);
	}
}

void MikanScene::removeMikanObject(MikanObjectPtr objectPtr)
{
	auto object_it= std::find(m_objects.begin(), m_objects.end(), objectPtr);
	if (object_it != m_objects.end())
	{
		MikanObjectPtr objectPtr= *object_it;

		// Remove renderable components from the GlScene
		std::vector<MikanSceneComponentPtr> sceneComponents;
		objectPtr->getComponentsOfType(sceneComponents);
		for (MikanSceneComponentPtr sceneComponent : sceneComponents)
		{
			IGlSceneRenderableConstPtr renderable= sceneComponent->getGlSceneRenderableConst();

			if (renderable)
			{
				m_glScene->removeInstance(renderable);
			}
		}

		// Forget about collidable components associated with the object
		std::vector<MikanColliderComponentPtr> colliderComponents;
		objectPtr->getComponentsOfType(colliderComponents);
		for (MikanColliderComponentPtr colliderComponent : colliderComponents)
		{
			for (auto collider_it = m_colliders.begin(); collider_it != m_colliders.end(); ++collider_it)
			{
				if (colliderComponent == collider_it->lock())
				{
					m_colliders.erase(collider_it);
					break;
				}
			}
		}

		m_objects.erase(object_it);
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