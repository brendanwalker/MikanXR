#pragma once

#include "GlShaderVar.h"

#include <string>
#include <vector>

struct GlUniform
{
	GlShaderVar var;
	int loc;
};

class GlUniformBlock
{
public:
	GlUniformBlock();
	GlUniformBlock(const std::string& name, int binding);
	~GlUniformBlock();

	void SetName(const std::string& name);
	const std::string& GetName() const;
	const std::vector<GlUniform>& GetUniforms() const;
	void SetBinding(int binding);
	int GetBinding() const;
	int size() const;

	void AddUniform(GlUniform& uniform);

private:
	std::string m_Name;
	std::vector<GlUniform> m_Uniforms;
	int m_Binding;
	int m_Size;
};
