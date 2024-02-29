#include "GlProgramConfig.h"
#include "GlProgram.h"

// -- CompositorLayerConfig ------
configuru::Config GlProgramConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt["materialName"]= materialName;
	pt["vertexShaderPath"]= vertexShaderPath.string();
	pt["fragmentShaderPath"]= fragmentShaderPath.string();

	CommonConfig::writeStdMap(pt, "uniformSemanticMap", uniformSemanticMap);

	return pt;
}

void GlProgramConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	materialName = pt.get_or<std::string>("materialName", materialName);
	vertexShaderPath = pt.get_or<std::string>("vertexShaderPath", vertexShaderPath.string());
	fragmentShaderPath = pt.get_or<std::string>("fragmentShaderPath", fragmentShaderPath.string());
	CommonConfig::readStdMap(pt, "uniformSemanticMap", uniformSemanticMap);
}

bool GlProgramConfig::loadGlProgramCode(GlProgramCode* outProgramCode)
{
	return outProgramCode->loadFromConfigData(
		getLoadedConfigPath(),
		vertexShaderPath,
		fragmentShaderPath,
		uniformSemanticMap);
}