#pragma once

#include "RendererFwd.h"

#include <memory>

#include "glm/ext/matrix_float4x4.hpp"

class IGlSceneRenderable
{
public:
	virtual IGlSceneRenderableConstPtr getConstSelfPointer() const = 0;
	virtual bool canCameraSee(GlCameraConstPtr renderingCamera) const = 0;
	virtual bool getVisible() const = 0;
	virtual void setVisible(bool bNewVisible) = 0;
	virtual const glm::mat4& getModelMatrix() const = 0;
	virtual const glm::mat4& getNormalMatrix() const = 0;
	virtual void setModelMatrix(const glm::mat4& mat) = 0;
	virtual GlMaterialInstancePtr getMaterialInstance() const = 0;
	virtual const GlMaterialInstanceConstPtr getMaterialInstanceConst() const = 0;
	virtual void render() const = 0;
};