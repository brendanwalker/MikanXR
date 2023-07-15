#include "Transform.h"

#include "MathGLM.h"
#include <glm/gtx/matrix_decompose.hpp>

GlmTransform::GlmTransform()
	: m_position(glm::vec3(0.f))
	, m_rotation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
	, m_mat(glm::mat4(1.f))
{
}

GlmTransform::GlmTransform(
	const glm::vec3& position)
	: m_position(position)
	, m_rotation(glm::quat())
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
{
	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::vec3& position, 
	const glm::quat& rotation)
	: m_position(position)
	, m_rotation(rotation)
	, m_scale(glm::vec3(1.f, 1.f, 1.f))
{
	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::vec3& position, 
	const glm::quat& rotation, 
	const glm::vec3& scale)
	: m_position(position)
	, m_rotation(rotation)
	, m_scale(scale)
{
	rebuildMat();
}

GlmTransform::GlmTransform(const glm::mat4& mat4)
	: m_position(glm::vec3(0.f))
	, m_rotation(glm::quat())
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
		m_rotation= orientation;
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

void GlmTransform::appendRotation(const glm::quat& deltaRotation)
{
	m_rotation= glm_composite_rotation(m_rotation, deltaRotation);
	rebuildMat();
}

void GlmTransform::appendTranslation(const glm::vec3& deltaPosition)
{
	m_position+= deltaPosition;
	rebuildMat();
}

void GlmTransform::rebuildMat()
{
	const glm::mat4 scale= glm::scale(glm::mat4(1.f), m_scale);
	const glm::mat4 rotation= glm::mat4_cast(m_rotation);
	const glm::mat4 translation= glm::translate(glm::mat4(1.f), m_position);

	m_mat= glm_composite_xform(glm_composite_xform(scale, rotation), translation);
}