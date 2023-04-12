#pragma once

#include "memory"
#include "vector"

#include "glm/ext/vector_int2_sized.hpp"
#include "glm/ext/vector_float4.hpp"

class GlCamera;
using GlCameraPtr = std::shared_ptr<GlCamera>;

class GlScene;
using GlScenePtr = std::shared_ptr<GlScene>;

class GlViewport
{
public:
	GlViewport();
	virtual ~GlViewport();

	void init();
	void update();
	void render();
	void dispose();

	void applyViewport();

	GlCameraPtr getCurrentCamera() const;
	GlCameraPtr pushCamera();
	void popCamera();

private:
	glm::i32vec2 m_origin;
	glm::i32vec2 m_size;
	glm::vec4 m_backgroundColor;

	std::vector<GlCameraPtr> m_cameraStack;

	GlScenePtr m_scene;
};
