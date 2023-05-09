#include "Transform.h"

#include <glm/gtx/matrix_decompose.hpp>

GlmTransform::GlmTransform()
	: m_position(glm::vec3(0.f))
	, m_orientation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
	, m_mat(glm::mat4(1.f))
{
}

GlmTransform::GlmTransform(
	const glm::vec3& position)
	: m_position(position)
	, m_orientation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
{
	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::vec3& position, 
	const glm::quat& orientation)
	: m_position(position)
	, m_orientation(orientation)
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
{
	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::vec3& position, 
	const glm::quat& orientation, 
	const glm::vec3& scale)
	: m_position(position)
	, m_orientation(orientation)
	, m_scale(scale)
{
	rebuildMat();
}

GlmTransform::GlmTransform(const glm::mat4& mat4)
	: m_position(glm::vec3(0.f))
	, m_orientation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
	, m_mat(glm::mat4(1.f))
{
	setMat4(mat4);
}

void GlmTransform::setMat4(const glm::mat4& mat4)
{
	// Extract position scale and rotation from the local transform
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	if (glm::decompose(
		mat4,
		scale, orientation, translation, skew, perspective))
	{
		m_position= translation;
		m_orientation= orientation;
		m_scale= scale;

		// Since the matrix could have had skew or perspective transforms in it
		// we have to rebuild it from just the decomposed TRS values
		rebuildMat();
	}
}

void GlmTransform::appendScale(const glm::vec3& deltaScale)
{
	m_scale+= deltaScale;
	rebuildMat();
}

void GlmTransform::appendOrientation(const glm::quat& deltaRotation)
{
	m_orientation= glm_composite_rotation(m_orientation, deltaRotation);
	rebuildMat();
}

void GlmTransform::appendPosition(const glm::vec3& deltaPosition)
{
	m_position+= deltaPosition;
	rebuildMat();
}

void GlmTransform::rebuildMat()
{
	m_mat= glm::mat4_cast(m_orientation) * glm::scale(glm::mat4(1.f), m_scale);
	m_mat= glm::translate(m_mat, m_position);
}