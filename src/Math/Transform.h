#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/ext/vector_float3.hpp"

class GlmTransform
{
public:
	GlmTransform();
	GlmTransform(const glm::vec3& position);
	GlmTransform(const glm::vec3& position, const glm::quat& orientation);
	GlmTransform(const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale);

	inline void getScale(const glm::vec3& scale) { m_scale= scale; rebuildMat(); }
	inline void getOrientation(const glm::quat& orientation) { m_orientation= orientation; rebuildMat(); }
	inline void getPosition(const glm::vec3& position) { m_position= position; rebuildMat(); }
	inline void setTransform(const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale)
	{
		m_position= position;
		m_orientation= orientation;
		m_scale= scale; 
		rebuildMat();
	}

	const glm::vec3& getScale() const { return m_scale; }
	const glm::quat& getOrientation() const { return m_orientation; }
	const glm::vec3& getPosition() const { return m_position; }
	const glm::mat4& getMat4() const { return m_mat; }

private:
	void rebuildMat();

	glm::vec3 m_position;
	glm::quat m_orientation;
	glm::vec3 m_scale;
	glm::mat4 m_mat;
};