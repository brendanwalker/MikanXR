#pragma once

#include "GlTypesFwd.h"

#include <string>

class GlShaderVar
{
protected:
	std::string m_Name;
	GLenum m_Type;
	int m_ArraySize;

public:
	GlShaderVar();
	GlShaderVar(GLenum type, const std::string& name);
	GlShaderVar(GLenum type, const std::string& name, int size);
	~GlShaderVar();

public:
	void SetName(const std::string& name);
	const std::string& GetName();
	void SetType(GLenum type);
	GLenum GetType();
	void SetArraySize(int size);
	int ArraySize();
};