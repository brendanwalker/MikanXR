#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"
#include "MkVertexConstants.h"

#include "stdint.h"
#include <string>
#include <vector>


class IMkVertexAttribute
{
public:
	virtual ~IMkVertexAttribute() {}

	virtual bool isCompatibleAttribute(IMkVertexAttributeConstPtr other) const = 0;
	virtual const std::string& getName() const = 0;
	virtual int getLocation() const = 0;
	virtual size_t getAttributeSize() const = 0;
	virtual size_t getOffset() const = 0;
	virtual eVertexSemantic getSemantic() const = 0;
	virtual eVertexDataType getDataType() const = 0;
	virtual bool getIsNormalized() const = 0;

	virtual void setLocation(int location) = 0;
	virtual void setOffset(size_t offset) = 0;
};

class IMkVertexDefinition
{
public:
	virtual ~IMkVertexDefinition() {}

	virtual bool getIsValid() const = 0;
	virtual const std::vector<IMkVertexAttributeConstPtr>& getAttributes() const = 0;
	virtual size_t getVertexSize() const  = 0;

	virtual void applyVertexDefintion() const = 0;
	virtual const std::string& getVertexDefinitionDesc() const = 0;
	virtual const IMkVertexAttribute* getFirstAttributeBySemantic(eVertexSemantic semantic) const = 0;
	virtual const IMkVertexAttribute* getAttributeByName(const std::string& name) const = 0;
	virtual bool isCompatibleDefinition(IMkVertexDefinitionConstPtr other) const = 0;
	virtual bool isCompatibleProgram(IMkShaderConstPtr program) const = 0;
};

MIKAN_RENDERER_FUNC(IMkVertexAttributePtr) createMkVertexAttribute(
	const std::string& name,
	eVertexDataType dataType,
	eVertexSemantic semantic,
	bool isNormalized = false);

MIKAN_RENDERER_FUNC(IMkVertexDefinitionPtr) createMkVertexDefinition(
	IMkVertexDefinitionConstPtr vertexDefinition);
MIKAN_RENDERER_FUNC(IMkVertexDefinitionPtr) createMkVertexDefinition(
	const std::vector<IMkVertexAttributeConstPtr>& attribtes);