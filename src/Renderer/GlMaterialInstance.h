#pragma once

#include "glm/ext/vector_float4.hpp"

class GlMaterialInstance
{
public:
	GlMaterialInstance();
	GlMaterialInstance(const class GlMaterial* material);

	const class GlMaterial* getMaterial() const { return m_material; }
	const glm::vec4& getDiffuseColor() const { return m_diffuseColor; }

	void setDiffuseColor(const glm::vec4& color) { m_diffuseColor= color; }
	void applyMaterialInstanceParameters();

private:
	const class GlMaterial* m_material = nullptr;
	glm::vec4 m_diffuseColor;
};
