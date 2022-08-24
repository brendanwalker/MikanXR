#include "GlMaterial.h"
#include "GlProgram.h"
#include "GlTexture.h"

GlMaterial::GlMaterial(
	const std::string& name, 
	GlProgram* program)
	: m_name(name)
	, m_program(program)
	, m_texture(nullptr)
{
}

bool GlMaterial::bindMaterial() const
{	
	if (m_program->bindProgram())
	{
		if (m_texture != nullptr)
		{
			m_texture->bindTexture();
		}

		return true;
	}

	return false;
}

void GlMaterial::unbindMaterial() const
{
	if (m_texture != nullptr)
	{
		m_texture->clearTexture();
	}

	m_program->unbindProgram();
}