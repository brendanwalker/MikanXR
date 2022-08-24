#include "GlShaderCache.h"
#include "GlStaticMeshInstance.h"
#include "GlProgram.h"

GlShaderCache* GlShaderCache::m_instance= nullptr;

GlShaderCache::GlShaderCache()
{
	m_instance= this;
}

GlShaderCache::~GlShaderCache()
{
	m_instance= nullptr;
}

bool GlShaderCache::startup()
{
	return true;
}

void GlShaderCache::shutdown()
{
	for (auto it = m_compileProgramCache.begin(); it != m_compileProgramCache.end(); ++it)
	{
		it->second->deleteProgram();
		delete it->second;
	}
	m_compileProgramCache.clear();
}

GlProgram* GlShaderCache::fetchCompiledGlProgram(
	const GlProgramCode* code)
{
	if (m_compileProgramCache.find(code->getCodeHash()) != m_compileProgramCache.end())
	{
		return m_compileProgramCache[code->getCodeHash()];
	}
	else
	{
		GlProgram* program = new GlProgram(*code);

		if (program->createProgram())
		{
			m_compileProgramCache[code->getCodeHash()] = program;
			return program;
		}
		else
		{
			delete program;
			return nullptr;
		}
	}
}