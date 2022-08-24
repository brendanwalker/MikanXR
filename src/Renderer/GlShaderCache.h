#pragma once

#include <string>
#include <map>

class GlShaderCache
{
public:
	GlShaderCache();
	virtual ~GlShaderCache();

	static GlShaderCache* getInstance() { return m_instance; }

	bool startup();
	void shutdown();

	class GlProgram* fetchCompiledGlProgram(const class GlProgramCode* code);

private:
	static GlShaderCache* m_instance;

	std::map<size_t, class GlProgram*> m_compileProgramCache;
};