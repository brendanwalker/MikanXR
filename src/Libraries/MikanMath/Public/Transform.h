#pragma once

#include "MikanMathExport.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/ext/vector_float3.hpp"

class MIKAN_MATH_CLASS GlmTransform
{
public:
	GlmTransform();
	GlmTransform(const glm::vec3& position);
	GlmTransform(const glm::vec3& position, const glm::quat& rotation);
	GlmTransform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
	GlmTransform(const glm::mat4& mat4);

	inline void setScale(const glm::vec3& scale) { m_scale= scale; rebuildMat(); }
	inline void setRotation(const glm::quat& orientation) { m_rotation= orientation; rebuildMat(); }
	inline void setPosition(const glm::vec3& position) { m_position= position; rebuildMat(); }
	inline void setTransform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
	{
		m_position= position;
		m_rotation= rotation;
		m_scale= scale; 
		rebuildMat();
	}
	void setMat4(const glm::mat4& mat4);

	void appendScale(const glm::vec3& deltaScale);
	void appendRotation(const glm::quat& deltaRotation);
	void appendTranslation(const glm::vec3& deltaPosition);

	const glm::vec3& getScale() const { return m_scale; }
	const glm::quat& getRotation() const { return m_rotation; }
	const glm::vec3& getPosition() const { return m_position; }
	const glm::mat4& getMat4() const { return m_mat; }

private:
	void rebuildMat();

	glm::vec3 m_position;
	glm::quat m_rotation;
	glm::vec3 m_scale;
	glm::mat4 m_mat;
};
