#include "MikanShaderConfig.h"

// -- GlVertexAttributeConfig ------
configuru::Config MikanVertexAttributeConfig::writeToJSON()
{
	configuru::Config pt= configuru::Config::object();

	pt["name"]= name;
	pt["dataType"]= VertexConstantUtils::vertexDataTypeToString(dataType);
	pt["semantic"]= VertexConstantUtils::vertexSemanticToString(semantic);

	return pt;
}

void MikanVertexAttributeConfig::readFromJSON(const configuru::Config& pt)
{
	name= pt.get_or<std::string>("name", name);

	std::string dataTypeString= pt.get_or<std::string>("dataType", "INVALID");
	dataType= VertexConstantUtils::vertexDataTypeFromString(dataTypeString);

	std::string semanticString= pt.get_or<std::string>("semantic", "INVALID");
	semantic= VertexConstantUtils::vertexSemanticFromString(semanticString);
}

// -- CompositorLayerConfig ------
configuru::Config MikanShaderConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt["materialName"]= materialName;
	pt["vertexShaderPath"]= vertexShaderPath.string();
	pt["fragmentShaderPath"]= fragmentShaderPath.string();
	CommonConfig::writeStdConfigVector(pt, "vertexAttributes", vertexAttributes);
	CommonConfig::writeStdMap(pt, "uniformSemanticMap", uniformSemanticMap);

	if (!domain.empty())
	{
		pt["domain"]= domain;
	}
	if (!vertexPreset.empty())
	{
		pt["vertexPreset"]= vertexPreset;
	}
	if (!sourceGraphPath.empty())
	{
		pt["sourceGraphPath"]= sourceGraphPath.generic_string();
	}

	return pt;
}

void MikanShaderConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	materialName= pt.get_or<std::string>("materialName", materialName);
	vertexShaderPath= pt.get_or<std::string>("vertexShaderPath", vertexShaderPath.string());
	fragmentShaderPath= pt.get_or<std::string>("fragmentShaderPath", fragmentShaderPath.string());
	CommonConfig::readStdConfigVector(pt, "vertexAttributes", vertexAttributes);
	CommonConfig::readStdMap(pt, "uniformSemanticMap", uniformSemanticMap);

	domain= pt.get_or<std::string>("domain", "");
	vertexPreset= pt.get_or<std::string>("vertexPreset", "");
	sourceGraphPath= pt.get_or<std::string>("sourceGraphPath", "");
}