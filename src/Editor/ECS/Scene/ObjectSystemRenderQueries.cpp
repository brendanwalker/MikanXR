#include "ColliderQuery.h"
#include "COlliderComponent.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MkScene.h"
#include "ObjectSystemRenderQueries.h"
#include "TransformComponent.h"

void addAllRenderablesToMkScene(std::set<const MikanObjectSystem*> objectSystems, IMkScenePtr mkScene,
								RenderableObjectFilter objectFilter)
{
	for (const MikanObjectSystem* objectSystem : objectSystems)
	{
		addAllRenderablesToMkScene(objectSystem->shared_from_this(), mkScene, objectFilter);
	}
}

void addAllRenderablesToMkScene(MikanObjectSystemConstPtr objectSystem, IMkScenePtr mkScene,
								RenderableObjectFilter objectFilter)
{
	objectSystem->visitAllObjects(
		[mkScene, objectFilter](MikanObjectPtr objectPtr)
		{
			if (objectFilter && !objectFilter(objectPtr))
				return;

			objectPtr->visitAllComponents(
				[mkScene](MikanComponentPtr componentPtr)
				{
					auto transformComponent= std::dynamic_pointer_cast<TransformComponent>(componentPtr);
					if (transformComponent)
					{
						auto renderable= transformComponent->getGlSceneRenderableConst();
						if (renderable)
						{
							mkScene->addInstance(renderable);
						}
					}
				});
		});
}