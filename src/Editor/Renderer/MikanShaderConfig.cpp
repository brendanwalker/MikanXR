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

	if (!uniformFloatDefaults.empty() || !uniformTextureDefaults.empty())
	{
		configuru::Config defaults= configuru::Config::object();
		for (const auto& [uniformName, values] : uniformFloatDefaults)
		{
			if (values.size() == 1)
			{
				defaults[uniformName]= values[0];
			}
			else
			{
				configuru::Config valueArray= configuru::Config::array();
				for (float value : values)
				{
					valueArray.push_back(value);
				}
				defaults[uniformName]= valueArray;
			}
		}
		for (const auto& [uniformName, texturePath] : uniformTextureDefaults)
		{
			defaults[uniformName]= texturePath;
		}
		pt["uniformDefaults"]= defaults;
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

	uniformFloatDefaults.clear();
	uniformTextureDefaults.clear();
	if (pt.has_key("uniformDefaults") && pt["uniformDefaults"].is_object())
	{
		for (const auto& entry : pt["uniformDefaults"].as_object())
		{
			const std::string& uniformName= entry.key();
			const configuru::Config& value= entry.value();
			if (value.is_string())
			{
				uniformTextureDefaults[uniformName]= value.as_string();
			}
			else if (value.is_number())
			{
				uniformFloatDefaults[uniformName]= {(float)value.as_double()};
			}
			else if (value.is_array())
			{
				std::vector<float>& values= uniformFloatDefaults[uniformName];
				for (const configuru::Config& component : value.as_array())
				{
					values.push_back((float)component.as_double());
				}
			}
		}
	}
}