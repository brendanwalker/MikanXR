#pragma once

#include "IMkSceneRenderable.h"
#include "GlProgramConstants.h"
#include "IMkScene.h"

#include <map>
#include <memory>
#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

struct GlDrawCall
{
	std::vector<IMkSceneRenderableConstWeakPtr> instances;
};
using GlDrawCallPtr = std::shared_ptr<GlDrawCall>;
using GlDrawCallConstPtr = std::shared_ptr<const GlDrawCall>;

class GlScene : public IMkScene, public std::enable_shared_from_this<GlScene>
{
public:
	const glm::vec4 k_clear_color = glm::vec4(0.447f, 0.565f, 0.604f, 1.0f);

	const float k_camera_vfov = 35.0f;
	const float k_camera_z_near = 0.1f;
	const float k_camera_z_far = 5000.0f;

	GlScene();
	virtual ~GlScene();

	virtual void addInstance(IMkSceneRenderableConstPtr instance) override;
	virtual void removeInstance(IMkSceneRenderableConstPtr instance) override;
	virtual void removeAllInstances() override;

	virtual void setLightColor(const glm::vec4& lightColor) override { m_lightColor= lightColor; }
	virtual const glm::vec4& getLightColor() const override { return m_lightColor; }

	virtual void setLightDirection(const glm::vec3& lightDirection) override { m_lightDirection = lightDirection; }
	virtual const glm::vec3& getLightDirection() const override { return m_lightDirection; }

	virtual void render(IMkCameraConstPtr camera, class GlStateStack& glStateStack) const override;

protected:
	eUniformBindResult materialBindCallback(
		IMkCameraConstPtr camera,
		GlProgramPtr program,
		eUniformDataType uniformDataType,
		eUniformSemantic uniformSemantic,
		const std::string& uniformName) const;
	eUniformBindResult materialInstanceBindCallback(
		IMkCameraConstPtr camera,
		IMkSceneRenderableConstPtr renderableInstance,
		GlProgramPtr program,
		eUniformDataType uniformDataType,
		eUniformSemantic uniformSemantic,
		const std::string& uniformName) const;

private:
	glm::vec4 m_lightColor;
	glm::vec3 m_lightDirection;

	class std::map<GlMaterialConstPtr, GlDrawCallPtr> m_drawCalls;
};