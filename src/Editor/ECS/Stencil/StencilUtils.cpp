#include "StencilUtils.h"
#include "BoxStencilSystem.h"
#include "BoxStencilComponent.h"
#include "ModelStencilSystem.h"
#include "ModelStencilComponent.h"
#include "ProjectManager.h"
#include "QuadStencilSystem.h"
#include "QuadStencilComponent.h"

namespace StencilUtils
{
	StencilComponentPtr getStencilById(ProjectManagerPtr projectManager, MikanStencilID stencilId)
	{
		// Try quad stencil system first
		QuadStencilSystemPtr quadStencilSystem = projectManager->getSystemOfType<QuadStencilSystem>();
		if (quadStencilSystem)
		{
			QuadStencilComponentPtr quadPtr = quadStencilSystem->getQuadStencilById(stencilId);
			if (quadPtr)
			{
				return quadPtr;
			}
		}

		// Then try box stencil system
		BoxStencilSystemPtr boxStencilSystem = projectManager->getSystemOfType<BoxStencilSystem>();
		if (boxStencilSystem)
		{
			BoxStencilComponentPtr boxPtr = boxStencilSystem->getBoxStencilById(stencilId);
			if (boxPtr)
			{
				return boxPtr;
			}
		}

		// Finally try model stencil system
		ModelStencilSystemPtr modelStencilSystem = projectManager->getSystemOfType<ModelStencilSystem>();
		if (modelStencilSystem)
		{
			ModelStencilComponentPtr modelPtr = modelStencilSystem->getModelStencilById(stencilId);
			if (modelPtr)
			{
				return modelPtr;
			}
		}

		return StencilComponentPtr();
	}

	eStencilType getStencilType(ProjectManagerPtr projectManager, MikanStencilID stencilId)
	{
		// Try quad stencil system first
		QuadStencilSystemPtr quadStencilSystem = projectManager->getSystemOfType<QuadStencilSystem>();
		if (quadStencilSystem)
		{
			QuadStencilComponentPtr quadPtr = quadStencilSystem->getQuadStencilById(stencilId);
			if (quadPtr)
			{
				return eStencilType::quad;
			}
		}

		// Then try box stencil system
		BoxStencilSystemPtr boxStencilSystem = projectManager->getSystemOfType<BoxStencilSystem>();
		if (boxStencilSystem)
		{
			BoxStencilComponentPtr boxPtr = boxStencilSystem->getBoxStencilById(stencilId);
			if (boxPtr)
			{
				return eStencilType::box;
			}
		}

		// Finally try model stencil system
		ModelStencilSystemPtr modelStencilSystem = projectManager->getSystemOfType<ModelStencilSystem>();
		if (modelStencilSystem)
		{
			ModelStencilComponentPtr modelPtr = modelStencilSystem->getModelStencilById(stencilId);
			if (modelPtr)
			{
				return eStencilType::model;
			}
		}

		return eStencilType::INVALID;
	}

	bool getStencilWorldTransform(ProjectManagerPtr projectManager, MikanStencilID parentStencilId, glm::mat4& outXform)
	{
		StencilComponentPtr stencilComponent = getStencilById(projectManager, parentStencilId);
		if (stencilComponent)
		{
			outXform = stencilComponent->getWorldTransform();
			return true;
		}

		return false;
	}

	bool removeStencil(ProjectManagerPtr projectManager, MikanStencilID stencilId)
	{
		eStencilType stencilType = getStencilType(projectManager, stencilId);

		switch (stencilType)
		{
		case eStencilType::quad:
		{
			QuadStencilSystemPtr quadStencilSystem = projectManager->getSystemOfType<QuadStencilSystem>();
			if (quadStencilSystem)
			{
				return quadStencilSystem->removeObject(stencilId);
			}
			break;
		}
		case eStencilType::box:
		{
			BoxStencilSystemPtr boxStencilSystem = projectManager->getSystemOfType<BoxStencilSystem>();
			if (boxStencilSystem)
			{
				return boxStencilSystem->removeObject(stencilId);
			}
			break;
		}
		case eStencilType::model:
		{
			ModelStencilSystemPtr modelStencilSystem = projectManager->getSystemOfType<ModelStencilSystem>();
			if (modelStencilSystem)
			{
				return modelStencilSystem->removeObject(stencilId);
			}
			break;
		}
		}

		return false;
	}
}
