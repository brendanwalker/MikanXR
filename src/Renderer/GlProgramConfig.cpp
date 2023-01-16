#include "GlProgramConfig.h"
#include "GlProgram.h"

// -- CompositorLayerConfig ------
const configuru::Config GlProgramConfig::writeToJSON()
{
	configuru::Config pt{
		{"materialName", materialName},
		{"vertexShaderPath", vertexShaderPath.string()},
		{"fragmentShaderPath", fragmentShaderPath.string()},
	};

	CommonConfig::writeStdMap(pt, "uniforms", uniforms);

	return pt;
}

void GlProgramConfig::readFromJSON(const configuru::Config& pt)
{
	materialName = pt.get_or<std::string>("materialName", materialName);
	vertexShaderPath = pt.get_or<std::string>("vertexShaderPath", vertexShaderPath.string());
	fragmentShaderPath = pt.get_or<std::string>("fragmentShaderPath", fragmentShaderPath.string());
	CommonConfig::readStdMap(pt, "uniforms", uniforms);
}

bool GlProgramConfig::loadGlProgramCode(GlProgramCode* outProgramCode)
{
	return outProgramCode->loadFromConfigData(
		getLoadedConfigPath(),
		vertexShaderPath,
		fragmentShaderPath,
		uniforms);
}