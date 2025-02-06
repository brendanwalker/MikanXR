#pragma once

#include "MkRendererFwd.h"

#include <memory>

#include "glm/ext/matrix_float4x4.hpp"

class IMkSceneRenderable
{
public:
	virtual IMkSceneRenderableConstPtr getConstSelfPointer() const = 0;
	virtual bool canCameraSee(IMkCameraConstPtr renderingCamera) const = 0;
	virtual bool getVisible() const = 0;
	virtual void setVisible(bool bNewVisible) = 0;
	virtual const glm::mat4& getModelMatrix() const = 0;
	virtual const glm::mat4& getNormalMatrix() const = 0;
	virtual void setModelMatrix(const glm::mat4& mat) = 0;
	virtual GlMaterialInstancePtr getMaterialInstance() const = 0;
	virtual const GlMaterialInstanceConstPtr getMaterialInstanceConst() const = 0;
	virtual void render() const = 0;
};