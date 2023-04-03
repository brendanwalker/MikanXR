#pragma once

#include "GlVertexDefinition.h"

#include <memory>

class IGlMesh
{
public:
	virtual void drawElements() const = 0;
	virtual bool createBuffers() = 0;
	virtual void deleteBuffers() = 0;

	virtual const GlVertexDefinition* getVertexDefinition() const = 0;
	virtual const uint8_t* getVertexData() const = 0;
	virtual const uint32_t getVertexCount() const = 0;

	virtual const uint8_t* getIndexData() const = 0;
	virtual const size_t getElementCount() const = 0;
	virtual const size_t getIndexPerElementCount() const = 0;
	virtual const size_t getIndexSize() const = 0;
};
typedef std::shared_ptr<IGlMesh> IGlMeshPtr;
typedef std::shared_ptr<const IGlMesh> IGlMeshConstPtr;