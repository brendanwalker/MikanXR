#include "Transform.h"

#include "MathGLM.h"
#include <glm/gtx/matrix_decompose.hpp>

struct GlmTransformData
{
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	glm::mat4 mat;
};

GlmTransform::GlmTransform()
	: m_data(new GlmTransformData())
{
	m_data->position= glm::vec3(0.f);
	m_data->rotation= glm::quat();
	m_data->scale= glm::vec3(1.f, 1.f, 1.f);
	m_data->mat= glm::mat4(1.f);
}

GlmTransform::GlmTransform(
	const glm::vec3& position)
	: m_data(new GlmTransformData())
{
	m_data->position = position;
	m_data->rotation = glm::quat();
	m_data->scale = glm::vec3(1.f, 1.f, 1.f);

	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::vec3& position, 
	const glm::quat& rotation)
	: m_data(new GlmTransformData())
{
	m_data->position = position;
	m_data->rotation = rotation;
	m_data->scale = glm::vec3(1.f, 1.f, 1.f);

	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::vec3& position, 
	const glm::quat& rotation, 
	const glm::vec3& scale)
	: m_data(new GlmTransformData())
{
	m_data->position = position;
	m_data->rotation = rotation;
	m_data->scale = scale;

	rebuildMat();
}

GlmTransform::GlmTransform(
	const glm::mat4& mat4)
	: m_data(new GlmTransformData())
{
	m_data->position = glm::vec3(0.f);
	m_data->rotation = glm::quat();
	m_data->scale = glm::vec3(1.f, 1.f, 1.f);

	setMat4(mat4);
}

GlmTransform::~GlmTransform()
{
	delete m_data;
}

const glm::vec3& GlmTransform::getScale() const 
{ 
	return m_data->scale; 
}

const glm::quat& GlmTransform::getRotation() const 
{ 
	return m_data->rotation; 
}

const glm::vec3& GlmTransform::getPosition() const 
{ 
	return m_data->position;
}

const glm::mat4& GlmTransform::getMat4() const
{ 
	return m_data->mat;
}

void GlmTransform::setScale(const glm::vec3& scale) 
{
	m_data->scale = scale; 
	rebuildMat(); 
}

void GlmTransform::setRotation(const glm::quat& orientation)
{ 
	m_data->rotation = orientation;
	rebuildMat(); 
}

void GlmTransform::setPosition(const glm::vec3& position)
{ 
	m_data->position = position;
	rebuildMat(); 
}

void GlmTransform::setTransform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
{
	m_data->position = position;
	m_data->rotation = rotation;
	m_data->scale = scale;
	rebuildMat();
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
		m_data->position= translation;
		m_data->rotation= orientation;
		m_data->scale= scale;

		// Since the matrix could have had skew or perspective transforms in it
		// we have to rebuild it from just the decomposed TRS values
		rebuildMat();
	}
}

void GlmTransform::appendScale(const glm::vec3& deltaScale)
{
	m_data->scale+= deltaScale;
	rebuildMat();
}

void GlmTransform::appendRotation(const glm::quat& deltaRotation)
{
	m_data->rotation= glm_composite_rotation(m_data->rotation, deltaRotation);
	rebuildMat();
}

void GlmTransform::appendTranslation(const glm::vec3& deltaPosition)
{
	m_data->position+= deltaPosition;
	rebuildMat();
}

void GlmTransform::rebuildMat()
{
	const glm::mat4 scale= glm::scale(glm::mat4(1.f), m_data->scale);
	const glm::mat4 rotation= glm::mat4_cast(m_data->rotation);
	const glm::mat4 translation= glm::translate(glm::mat4(1.f), m_data->position);

	m_data->mat= glm_composite_xform(glm_composite_xform(scale, rotation), translation);
}