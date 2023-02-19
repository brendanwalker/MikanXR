#pragma once

#include <memory>
#include <string>
#include <map>

//-- typedefs -----
class GlProgramCode;
class GlProgram;
typedef std::shared_ptr<GlProgram> GlProgramPtr;

class GlShaderCache
{
public:
	GlShaderCache();
	virtual ~GlShaderCache();

	static GlShaderCache* getInstance() { return m_instance; }

	bool startup();
	void shutdown();

	GlProgramPtr fetchCompiledGlProgram(const GlProgramCode* code);

private:
	static GlShaderCache* m_instance;

	std::map<std::string, GlProgramPtr> m_compileProgramCache;
};