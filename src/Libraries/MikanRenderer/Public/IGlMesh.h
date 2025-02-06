#pragma once

#include "GlVertexDefinition.h"

#include <memory>
#include <string>

class IGlMesh
{
public:
	virtual void drawElements() const = 0;
	virtual bool createResources() = 0;
	virtual void deleteResources() = 0;

	virtual std::string getName() const = 0;
	virtual class IMkWindow* getOwnerWindow() const = 0;
	virtual std::shared_ptr<class GlMaterialInstance> getMaterialInstance() const = 0;
	virtual const uint8_t* getVertexData() const = 0;
	virtual const uint32_t getVertexCount() const = 0;

	virtual const uint8_t* getIndexData() const = 0;
	virtual const size_t getElementCount() const = 0;
	virtual const size_t getIndexPerElementCount() const = 0;
	virtual const size_t getIndexSize() const = 0;
};
typedef std::shared_ptr<IGlMesh> IGlMeshPtr;
typedef std::shared_ptr<const IGlMesh> IGlMeshConstPtr;