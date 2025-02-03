#pragma once

#include "IGLSceneRenderable.h"
#include "GlProgramConstants.h"

#include <map>
#include <memory>
#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

struct GlDrawCall
{
	std::vector<IGlSceneRenderableConstWeakPtr> instances;
};
using GlDrawCallPtr = std::shared_ptr<GlDrawCall>;
using GlDrawCallConstPtr = std::shared_ptr<const GlDrawCall>;

class GlScene : public std::enable_shared_from_this<GlScene>
{
public:
	const glm::vec4 k_clear_color = glm::vec4(0.447f, 0.565f, 0.604f, 1.0f);

	const float k_camera_vfov = 35.0f;
	const float k_camera_z_near = 0.1f;
	const float k_camera_z_far = 5000.0f;

	GlScene();
	virtual ~GlScene();

	void addInstance(IGlSceneRenderableConstPtr instance);
	void removeInstance(IGlSceneRenderableConstPtr instance);
	void removeAllInstances();

	void setLightColor(const glm::vec4& lightColor) { m_lightColor= lightColor; }
	const glm::vec4& getLightColor() const { return m_lightColor; }

	void setLightDirection(const glm::vec3& lightDirection) { m_lightDirection = lightDirection; }
	const glm::vec3& getLightDirection() const { return m_lightDirection; }

	void render(GlCameraConstPtr camera, class GlStateStack& glStateStack) const;

protected:
	eUniformBindResult materialBindCallback(
		GlCameraConstPtr camera,
		GlProgramPtr program,
		eUniformDataType uniformDataType,
		eUniformSemantic uniformSemantic,
		const std::string& uniformName) const;
	eUniformBindResult materialInstanceBindCallback(
		GlCameraConstPtr camera,
		IGlSceneRenderableConstPtr renderableInstance,
		GlProgramPtr program,
		eUniformDataType uniformDataType,
		eUniformSemantic uniformSemantic,
		const std::string& uniformName) const;

private:
	glm::vec4 m_lightColor;
	glm::vec3 m_lightDirection;

	class std::map<GlMaterialConstPtr, GlDrawCallPtr> m_drawCalls;
};