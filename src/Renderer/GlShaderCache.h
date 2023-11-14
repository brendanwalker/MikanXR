#pragma once

#include "RendererFwd.h"

#include <memory>
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

	GlProgramPtr allocateEmptyGlProgram(const std::string& programName);
	void removeGlProgramFromCache(GlProgramPtr program);
	GlProgramPtr fetchCompiledGlProgram(const GlProgramCode* code);

private:
	static GlShaderCache* m_instance;

	std::map<std::string, GlProgramPtr> m_programCache;
};