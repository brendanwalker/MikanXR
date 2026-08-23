#include "ShapeUtils.h"
#include "BoxShapeSystem.h"
#include "BoxShapeComponent.h"
#include "ModelShapeSystem.h"
#include "ModelShapeComponent.h"
#include "ProjectManager.h"
#include "QuadShapeSystem.h"
#include "QuadShapeComponent.h"

namespace ShapeUtils
{
ShapeComponentPtr getShapeById(ProjectManagerPtr projectManager, MikanShapeID shapeId)
{
	QuadShapeSystemPtr quadSystem= projectManager->getSystemOfType<QuadShapeSystem>();
	if (quadSystem)
	{
		QuadShapeComponentPtr ptr= quadSystem->getQuadShapeById(shapeId);
		if (ptr)
			return ptr;
	}

	BoxShapeSystemPtr boxSystem= projectManager->getSystemOfType<BoxShapeSystem>();
	if (boxSystem)
	{
		BoxShapeComponentPtr ptr= boxSystem->getBoxShapeById(shapeId);
		if (ptr)
			return ptr;
	}

	ModelShapeSystemPtr modelSystem= projectManager->getSystemOfType<ModelShapeSystem>();
	if (modelSystem)
	{
		ModelShapeComponentPtr ptr= modelSystem->getModelShapeById(shapeId);
		if (ptr)
			return ptr;
	}

	return ShapeComponentPtr();
}

eShapeType getShapeType(ProjectManagerPtr projectManager, MikanShapeID shapeId)
{
	QuadShapeSystemPtr quadSystem= projectManager->getSystemOfType<QuadShapeSystem>();
	if (quadSystem && quadSystem->getQuadShapeById(shapeId))
		return eShapeType::quad;

	BoxShapeSystemPtr boxSystem= projectManager->getSystemOfType<BoxShapeSystem>();
	if (boxSystem && boxSystem->getBoxShapeById(shapeId))
		return eShapeType::box;

	ModelShapeSystemPtr modelSystem= projectManager->getSystemOfType<ModelShapeSystem>();
	if (modelSystem && modelSystem->getModelShapeById(shapeId))
		return eShapeType::model;

	return eShapeType::INVALID;
}

bool removeShape(ProjectManagerPtr projectManager, MikanShapeID shapeId)
{
	switch (getShapeType(projectManager, shapeId))
	{
	case eShapeType::quad:
	{
		auto system= projectManager->getSystemOfType<QuadShapeSystem>();
		return system ? system->removeObjectByPrimaryComponentId(shapeId) : false;
	}
	case eShapeType::box:
	{
		auto system= projectManager->getSystemOfType<BoxShapeSystem>();
		return system ? system->removeObjectByPrimaryComponentId(shapeId) : false;
	}
	case eShapeType::model:
	{
		auto system= projectManager->getSystemOfType<ModelShapeSystem>();
		return system ? system->removeObjectByPrimaryComponentId(shapeId) : false;
	}
	default:
		return false;
	}
}
} // namespace ShapeUtils
