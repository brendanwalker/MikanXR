#pragma once

#include <memory>

#include "glm/ext/matrix_float4x4.hpp"

class GlMaterialInstance;
typedef std::shared_ptr<GlMaterialInstance> GlMaterialInstancePtr;
typedef std::shared_ptr<const GlMaterialInstance> GlMaterialInstanceConstPtr;

class IGlSceneRenderable
{
public:
	virtual bool getVisible() const = 0;
	virtual const glm::mat4& getModelMatrix() const = 0;
	virtual GlMaterialInstancePtr getMaterialInstance() const = 0;
	virtual const GlMaterialInstanceConstPtr getMaterialInstanceConst() const = 0;
	virtual void render() const = 0;
};