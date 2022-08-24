#include "GlMaterialInstance.h"
#include "GlMaterial.h"
#include "GlProgram.h"

GlMaterialInstance::GlMaterialInstance()
	: m_material(nullptr)
	, m_diffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
{
}

GlMaterialInstance::GlMaterialInstance(const GlMaterial* material)
	: m_material(material)
	, m_diffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
{
}

void GlMaterialInstance::applyMaterialInstanceParameters()
{
	m_material->getProgram()->setVector4Uniform(eUniformSemantic::diffuseColorRGBA, m_diffuseColor);
}