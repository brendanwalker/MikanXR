#pragma once

#include "IGLSceneRenderable.h"
#include "RendererFwd.h"

#include <map>
#include <vector>

#include "glm/ext/vector_float4.hpp"

struct GlDrawCall
{
	std::vector<IGlSceneRenderableConstPtr> instances;
};

class GlScene
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

	void render() const;

private:
	class std::map<GlMaterialConstPtr, GlDrawCall*> m_drawCalls;
};