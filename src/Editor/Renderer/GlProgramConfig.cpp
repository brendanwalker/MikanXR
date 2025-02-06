#include "GlProgramConfig.h"

// -- GlVertexAttributeConfig ------
configuru::Config GlVertexAttributeConfig::writeToJSON()
{
	configuru::Config pt;

	pt["name"]= name;
	pt["dataType"]= VertexConstantUtils::vertexDataTypeToString(dataType);
	pt["semantic"]= VertexConstantUtils::vertexSemanticToString(semantic);

	return pt;
}

void GlVertexAttributeConfig::readFromJSON(const configuru::Config& pt)
{
	name= pt.get_or<std::string>("name", name);

	std::string dataTypeString = pt.get_or<std::string>("dataType", "INVALID");
	dataType= VertexConstantUtils::vertexDataTypeFromString(dataTypeString);
	
	std::string semanticString = pt.get_or<std::string>("semantic", "INVALID");
	semantic= VertexConstantUtils::vertexSemanticFromString(semanticString);
}

// -- CompositorLayerConfig ------
configuru::Config GlProgramConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt["materialName"]= materialName;
	pt["vertexShaderPath"]= vertexShaderPath.string();
	pt["fragmentShaderPath"]= fragmentShaderPath.string();
	CommonConfig::writeStdConfigVector(pt, "vertexAttributes", vertexAttributes);
	CommonConfig::writeStdMap(pt, "uniformSemanticMap", uniformSemanticMap);

	return pt;
}

void GlProgramConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	materialName = pt.get_or<std::string>("materialName", materialName);
	vertexShaderPath = pt.get_or<std::string>("vertexShaderPath", vertexShaderPath.string());
	fragmentShaderPath = pt.get_or<std::string>("fragmentShaderPath", fragmentShaderPath.string());
	CommonConfig::readStdConfigVector(pt, "vertexAttributes", vertexAttributes);
	CommonConfig::readStdMap(pt, "uniformSemanticMap", uniformSemanticMap);
}