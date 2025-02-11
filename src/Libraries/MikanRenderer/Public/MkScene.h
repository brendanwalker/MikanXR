#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"
#include "MkMaterial.h"
#include "MkShaderConstants.h"
#include "IMkScene.h"

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

class MIKAN_RENDERER_CLASS MkScene : public IMkScene
{
public:
	MkScene();
	virtual ~MkScene();

	virtual void addInstance(IMkSceneRenderableConstPtr instance) override;
	virtual void removeInstance(IMkSceneRenderableConstPtr instance) override;
	virtual void removeAllInstances() override;

	virtual void setLightColor(const glm::vec4& lightColor) override;
	virtual const glm::vec4& getLightColor() const override;

	virtual void setLightDirection(const glm::vec3& lightDirection) override;
	virtual const glm::vec3& getLightDirection() const override;

	virtual void render(IMkCameraConstPtr camera, class MkStateStack& MkStateStack) const override;

protected:
	eUniformBindResult materialBindCallback(
		IMkCameraConstPtr camera,
		IMkShaderPtr program,
		eUniformDataType uniformDataType,
		eUniformSemantic uniformSemantic,
		const std::string& uniformName) const;
	eUniformBindResult materialInstanceBindCallback(
		IMkCameraConstPtr camera,
		IMkSceneRenderableConstPtr renderableInstance,
		IMkShaderPtr program,
		eUniformDataType uniformDataType,
		eUniformSemantic uniformSemantic,
		const std::string& uniformName) const;

private:
	struct MkSceneImpl* m_impl;
};