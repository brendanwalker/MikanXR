#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

namespace ShapeUtils
{
	ShapeComponentPtr getShapeById(ProjectManagerPtr projectManager, MikanShapeID shapeId);
	eShapeType getShapeType(ProjectManagerPtr projectManager, MikanShapeID shapeId);
	bool removeShape(ProjectManagerPtr projectManager, MikanShapeID shapeId);
}
