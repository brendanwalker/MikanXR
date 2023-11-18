#pragma once

#include "GlTypesFwd.h"
#include "GlShaderVar.h"

#include <vector>

class GlBufferBlock
{
public:
	GlBufferBlock();
	GlBufferBlock(const std::string& name, int binding);
	~GlBufferBlock();

	void SetName(const std::string& name);
	std::string GetName() const;
	const std::vector<GlShaderVar>& GetVars() const;
	void SetBinding(int binding);
	int GetBinding() const;
	int size() const;

	void AddVar(GlShaderVar var);

private:
	std::string m_Name;
	std::vector<GlShaderVar> m_Vars;
	int m_Binding;
	int m_Size;
};
