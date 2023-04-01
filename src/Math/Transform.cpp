#include "Transform.h"

GlmTransform::GlmTransform()
	: m_position(glm::vec3(0.f))
	, m_orientation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
	, m_mat(glm::mat4(1.f))
{
}

GlmTransform::GlmTransform(
	const glm::vec3& position)
	: m_position(glm::vec3(0.f))
	, m_orientation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
{
	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::vec3& position, 
	const glm::quat& orientation)
	: m_position(glm::vec3(0.f))
	, m_orientation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
{
	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::vec3& position, 
	const glm::quat& orientation, 
	const glm::vec3& scale)
	: m_position(glm::vec3(0.f))
	, m_orientation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
{
	rebuildMat();
}

void GlmTransform::rebuildMat()
{
	m_mat= glm::mat4_cast(m_orientation) * glm::scale(glm::mat4(1.f), m_scale);
	m_mat= glm::translate(m_mat, m_position);
}