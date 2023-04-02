#pragma once

#include "GlVertexDefinition.h"

#include <memory>

class IGlMesh
{
public:
	virtual void drawElements() const = 0;
	virtual bool createBuffers() = 0;
	virtual void deleteBuffers() = 0;
};
typedef std::shared_ptr<IGlMesh> IGlMeshPtr;
typedef std::shared_ptr<const IGlMesh> IGlMeshConstPtr;