#include "SceneActorParenting.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "ShapeComponent.h"
#include "StencilComponent.h"

bool isValidSceneActorParent(TransformComponentConstPtr parentComponent)
{
	return std::dynamic_pointer_cast<const SceneComponent>(parentComponent) != nullptr
		   || std::dynamic_pointer_cast<const AnchorComponent>(parentComponent) != nullptr
		   || std::dynamic_pointer_cast<const StencilComponent>(parentComponent) != nullptr
		   || std::dynamic_pointer_cast<const ShapeComponent>(parentComponent) != nullptr;
}
