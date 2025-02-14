#pragma once

#include "IMkSceneRenderable.h"
#include "MkRendererFwd.h"
#include "MkRendererExport.h"

#include <string>

class IMkStaticMeshInstance : public IMkSceneRenderable
{
public:
	virtual ~IMkStaticMeshInstance() {}

	virtual const std::string& getName() const = 0;
	virtual void bindToScene(IMkScenePtr scene) = 0;
	virtual void removeFromBoundScene() = 0;
	virtual void setIsVisibleToCamera(const std::string& cameraName, bool bVisible) = 0;
	virtual IMkMeshConstPtr getMesh() const = 0;
};

MIKAN_RENDERER_FUNC(IMkStaticMeshInstancePtr) createMkStaticMeshInstance(
	const std::string& name,
	IMkMeshConstPtr mesh);
