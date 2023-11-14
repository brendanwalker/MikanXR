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
	m_programCache.clear();
	m_instance = nullptr;
}

GlProgramPtr GlShaderCache::allocateEmptyGlProgram(const std::string& programName)
{
	auto it = m_programCache.find(programName);
	if (it != m_programCache.end())
	{
		// Nuke any existing program using the same name
		m_programCache.erase(it);
	}

	// Create a new empty program
	GlProgramPtr program = std::make_shared<GlProgram>(programName);
	m_programCache[programName] = program;
	return program;
}

GlProgramPtr GlShaderCache::fetchCompiledGlProgram(
	const GlProgramCode* code)
{
	auto it = m_programCache.find(code->getProgramName());
	if (it != m_programCache.end())
	{
		GlProgramPtr existingProgram= it->second;

		if (existingProgram->getProgramCode().getCodeHash() == code->getCodeHash())
		{
			// Found a compiled version of the code
			return existingProgram;
		}
		else
		{
			// Old compiled program is stale so delete it
			m_programCache.erase(it);
		}
	}

	// (Re)compile program and add it to the cache
	GlProgramPtr program = std::make_shared<GlProgram>(*code);
	if (program->compileProgram())
	{
		m_programCache[code->getProgramName()] = program;
		return program;
	}
	else
	{
		// Clean up the program if it failed to compile
		return nullptr;
	}
}

void GlShaderCache::removeGlProgramFromCache(GlProgramPtr program)
{
	auto it = m_programCache.find(program->getProgramCode().getProgramName());
	if (it != m_programCache.end())
	{
		m_programCache.erase(it);
	}
}