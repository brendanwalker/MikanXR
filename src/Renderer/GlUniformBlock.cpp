#include "GlUniformBlock.h"

#include <GL/glew.h>

GlUniformBlock::GlUniformBlock() :
	m_Name(""),
	m_Binding(-1),
	m_Size(0)
{}

GlUniformBlock::GlUniformBlock(const std::string& name, int binding) :
	m_Name(name),
	m_Binding(binding),
	m_Size(0)
{}

GlUniformBlock::~GlUniformBlock()
{}

void GlUniformBlock::SetName(const std::string& name)
{
	m_Name = name;
}

const std::string& GlUniformBlock::GetName() const
{
	return m_Name;
}

const std::vector<GlUniform>& GlUniformBlock::GetUniforms() const
{
	return m_Uniforms;
}

void GlUniformBlock::SetBinding(int binding)
{
	m_Binding = binding;
}

int GlUniformBlock::GetBinding() const
{
	return m_Binding;
}

int GlUniformBlock::size() const
{
	return m_Size;
}

void GlUniformBlock::AddUniform(GlUniform& uniform)
{
	std::string name = uniform.var.GetName();
	uniform.var.SetName(name.substr(name.find_first_of(".") + 1).c_str());
	m_Uniforms.push_back(uniform);

	switch (uniform.var.GetType())
	{
		case GL_BOOL:
		case GL_INT:
		case GL_UNSIGNED_INT:
			m_Size += sizeof(int);
			break;
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
		case GL_UNSIGNED_INT_VEC2:
			m_Size += sizeof(int) * 2;
			break;
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
		case GL_UNSIGNED_INT_VEC3:
			m_Size += sizeof(int) * 3;
			break;
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
		case GL_UNSIGNED_INT_VEC4:
			m_Size += sizeof(int) * 4;
			break;

		case GL_FLOAT:
			m_Size += sizeof(float);
			break;
		case GL_FLOAT_VEC2:
			m_Size += sizeof(float) * 2;
			break;
		case GL_FLOAT_VEC3:
			m_Size += sizeof(float) * 3;
			break;
		case GL_FLOAT_VEC4:
			m_Size += sizeof(float) * 4;
			break;
	}
}
