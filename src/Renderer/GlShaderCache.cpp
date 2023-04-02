#include "GlShaderCache.h"
#include "GlProgram.h"

GlShaderCache* GlShaderCache::m_instance= nullptr;

GlShaderCache::GlShaderCache()
{
}

GlShaderCache::~GlShaderCache()
{
}

bool GlShaderCache::startup()
{
	m_instance = this;

	return true;
}

void GlShaderCache::shutdown()
{
	m_compileProgramCache.clear();
	m_instance = nullptr;
}

GlProgramPtr GlShaderCache::fetchCompiledGlProgram(
	const GlProgramCode* code)
{
	auto it = m_compileProgramCache.find(code->getProgramName());
	if (it != m_compileProgramCache.end())
	{
		GlProgramPtr compiledProgram= it->second;

		if (compiledProgram->getProgramCode().getCodeHash() == code->getCodeHash())
		{
			// Found a compiled version of the code
			return compiledProgram;
		}
		else
		{
			// Old compiled program is stale so delete it
			m_compileProgramCache.erase(it);
		}
	}

	// (Re)compile program and add it to the cache
	GlProgramPtr program = std::make_shared<GlProgram>(*code);
	if (program->createProgram())
	{
		m_compileProgramCache[code->getProgramName()] = program;
		return program;
	}
	else
	{
		// Clean up the program if it failed to compile
		return nullptr;
	}
}