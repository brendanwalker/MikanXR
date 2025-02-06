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
	virtual ~GlmTransform();

	void setScale(const glm::vec3& scale);
	void setRotation(const glm::quat& orientation);
	void setPosition(const glm::vec3& position);
	void setTransform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
	void setMat4(const glm::mat4& mat4);

	void appendScale(const glm::vec3& deltaScale);
	void appendRotation(const glm::quat& deltaRotation);
	void appendTranslation(const glm::vec3& deltaPosition);

	const glm::vec3& getScale() const;
	const glm::quat& getRotation() const;
	const glm::vec3& getPosition() const;
	const glm::mat4& getMat4() const;

private:
	void rebuildMat();
	struct GlmTransformData* m_data;
};
