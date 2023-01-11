#include "GlProgramConfig.h"
#include "GlProgram.h"

// -- CompositorLayerConfig ------
GlProgramConfig::GlProgramConfig(const std::string& program_name)
	: CommonConfig(program_name)
{
}

const configuru::Config GlProgramConfig::writeToJSON() const
{
	configuru::Config pt{
		{"materialName", materialName},
		{"vertexShaderPath", vertexShaderPath.string()},
		{"fragmentShaderPath", fragmentShaderPath.string()},
		{"uniforms", uniforms},
	};

	return pt;
}

void GlProgramConfig::readFromJSON(const configuru::Config& pt)
{
	materialName = pt.get_or<std::string>("materialName", materialName);
	vertexShaderPath = pt.get_or<std::string>("vertexShaderPath", vertexShaderPath.string());
	fragmentShaderPath = pt.get_or<std::string>("fragmentShaderPath", fragmentShaderPath.string());
	uniforms = pt.get_or< std::map<std::string, std::string> >("uniforms", uniforms);
}