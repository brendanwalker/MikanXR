#include "GlViewport.h"
#include "GlCommon.h"
#include "GlCamera.h"
#include "GlScene.h"
#include "Colors.h"

GlViewport::GlViewport()
	: m_origin(glm::i32vec2(0, 0))
	, m_size(glm::i32vec2(0, 0))
	, m_backgroundColor(Colors::CornflowerBlue, 1.f)
	, m_scene(std::make_shared<GlScene>())
{

}

GlViewport::~GlViewport()
{
	dispose();
}

void GlViewport::init()
{

}

void GlViewport::update()
{

}

void GlViewport::render()
{
	m_scene->render();
}

void GlViewport::dispose()
{

}

void GlViewport::applyViewport()
{
	glClearColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
	glViewport(m_origin.x, m_origin.y, m_size.x, m_size.y);
}

GlCameraPtr GlViewport::getCurrentCamera() const
{
	return m_cameraStack.size() > 0 ? m_cameraStack[m_cameraStack.size() - 1] : GlCameraPtr();
}

GlCameraPtr GlViewport::pushCamera()
{
	GlCameraPtr newCamera = std::make_shared<GlCamera>();
	m_cameraStack.push_back(newCamera);

	return newCamera;
}

void GlViewport::popCamera()
{
	if (m_cameraStack.size() > 0)
	{
		m_cameraStack.pop_back();
	}
}