#include "ColliderQuery.h"
#include "COlliderComponent.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MkScene.h"
#include "ObjectSystemRenderQueries.h"
#include "TransformComponent.h"

void addAllRenderablesToMkScene(std::set<const MikanObjectSystem*> objectSystems, IMkScenePtr mkScene)
{
	for (const MikanObjectSystem* objectSystem : objectSystems)
	{
		addAllRenderablesToMkScene(objectSystem->shared_from_this(), mkScene);
	}
}

void addAllRenderablesToMkScene(MikanObjectSystemConstPtr objectSystem, IMkScenePtr mkScene)
{
	objectSystem->visitAllObjects(
		[mkScene](MikanObjectPtr objectPtr)
		{
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