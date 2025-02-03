#include "GlShaderVar.h"

#include <GL/glew.h>

GlShaderVar::GlShaderVar() :
	m_Name(""),
	m_Type(GL_BOOL),
	m_ArraySize(1)
{}

GlShaderVar::GlShaderVar(GLenum type, const std::string& name) :
	m_Name(name),
	m_Type(type),
	m_ArraySize(1)
{
	if (m_Name[m_Name.size() - 1] == ']')
		m_Name = m_Name.substr(0, m_Name.find_last_of('['));
}

GlShaderVar::GlShaderVar(GLenum type, const std::string& name, int size) :
	m_Name(name),
	m_Type(type),
	m_ArraySize(size)
{
	if (m_Name[m_Name.size() - 1] == ']')
		m_Name = m_Name.substr(0, m_Name.find_last_of('['));
}

GlShaderVar::~GlShaderVar()
{}

void GlShaderVar::SetName(const std::string& name)
{
	m_Name = name;
	if (m_Name[m_Name.size() - 1] == ']')
		m_Name = m_Name.substr(0, m_Name.find_last_of('['));
}

const std::string& GlShaderVar::GetName() const
{
	return m_Name;
}

void GlShaderVar::SetType(GLenum type)
{
	m_Type = type;
}

GLenum GlShaderVar::GetType() const
{
	return m_Type;
}

void GlShaderVar::SetArraySize(int size)
{
	m_ArraySize = size;
}

int GlShaderVar::ArraySize() const
{
	return m_ArraySize;
}
