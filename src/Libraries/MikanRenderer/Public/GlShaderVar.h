#pragma once

#include "GlTypesFwd.h"

#include <string>

class GlShaderVar
{
public:
	GlShaderVar();
	GlShaderVar(GLenum type, const std::string& name);
	GlShaderVar(GLenum type, const std::string& name, int size);
	~GlShaderVar();

	void SetName(const std::string& name);
	const std::string& GetName() const;
	void SetType(GLenum type);
	GLenum GetType() const;
	void SetArraySize(int size);
	int ArraySize() const;

protected:
	std::string m_Name;
	GLenum m_Type;
	int m_ArraySize;
};